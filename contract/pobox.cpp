/*
Copyright 2019 cc32d9@gmail.com

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

#include <eosio/eosio.hpp>
#include <eosio/multi_index.hpp>
#include <eosio/crypto.hpp>
#include <eosio/system.hpp>


using namespace eosio;
using std::vector;

CONTRACT pobox : public eosio::contract {
 public:
  pobox( name self, name code, datastream<const char*> ds ):
    contract(self, code, ds)
    {}
  
  ACTION send(name from, name to, vector<char> iv, public_key ephem_key,
              vector<char> ciphertext, checksum256 mac)
  {
    require_auth(from);
    check(is_account(to), "to account does not exist");
    require_recipient(to);

    int64_t msgid;
    msgcounters _cntr(_self, 0);
    auto cntitr = _cntr.find(to.value);
    if( cntitr == _cntr.end() ) {
      msgid = 1;
      _cntr.emplace(from, [&]( auto& item ) {
                            item.recipient = to;
                            item.count = msgid;
                          });
    }
    else {
      msgid = cntitr->count + 1;
      _cntr.modify( *cntitr, same_payer, [&]( auto& item ) {
                                           item.count = msgid;
                                         });
    }
      
    messages _msg(_self, to.value);
    _msg.emplace(from, [&]( auto& item ) {
                         item.id = msgid;
                          item.sender = from;
                          item.iv = iv;
                          item.ephem_key = ephem_key;
                          item.ciphertext = ciphertext;
                          item.mac = mac;
                        });
    
  }

  
  ACTION unsend(name from, name to, uint64_t id)
  {
    require_auth(from);
    check(is_account(to), "to account does not exist");
    require_recipient(to);
    messages _msg(_self, to.value);
    auto itr = _msg.find(id);
    check(itr != _msg.end(), "Cannot find the message");
    _msg.erase(itr);
  }

  
  ACTION accept(name recipient, uint64_t id, int32_t count)
  {
    require_auth(recipient);
    messages _msg(_self, recipient.value);
    bool did_something = false;
    auto itr = _msg.begin();
    while( itr->id <= id && count-- > 0 ) {
      itr = _msg.erase(itr);
      did_something = true;
    }

    check(did_something, "no messages to accept");
  }
    
     
 private:

  struct [[eosio::table("msgcounters")]] msgcounter {
    name             recipient;
    uint64_t         count;
    auto primary_key()const { return recipient.value; }
  };
  
  typedef eosio::multi_index<name("msgcounters"), msgcounter> msgcounters;

  
  struct [[eosio::table("messages")]] message {
    uint64_t         id;             /* autoincrement */
    name             sender;
    vector<char>  iv;
    public_key       ephem_key;
    vector<char>  ciphertext;
    checksum256      mac;

    auto primary_key()const { return id; }
  };
  EOSLIB_SERIALIZE(message, (id)(sender)(iv)(ephem_key)(ciphertext)(mac));
  
  typedef eosio::multi_index<name("messages"), message> messages;  
};

  
