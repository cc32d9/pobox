# P.O. Box demo

This is a simple demo that allows anyone on eosio blockchain send an
encrypted message to any account, and it can only be decrypted with
the recipient's private key.

The smart contract is deployed under `poboxxxxxxxx` on WAX Testnet. It
stores the encrypted messages in "messages" table, with recipient name
as scope. It also allows the recipient to "accept" messages, deleting
them from the table.

The two utilities in "nodejs" folder are for sending and decoding the
messages. The sending script looks for permission called "pobox" on
the recipient account, and takes its public key for encryption. If
it's not found, it uses the recipient's active key.


```
# Send the content of mysecretmessage.txt to poboxrcpnt11

./bin/pobox_send --url=https://testnet.waxsweden.org --sender poboxsender1 \
  --senderkey 5Jxxxx --recipient poboxrcpnt11 --contract poboxxxxxxxx \
  --file mysecretmessage.txt


# poboxrcpnt11 can decrypt all incoming messages by specifying the private key:

./bin/pobox_read --url=https://testnet.waxsweden.org --recipient poboxrcpnt11 \
  --key 5Kxxxxxxx --contract poboxxxxxxxx --dir /tmp/pobox_incoming


# poboxrcpnt11 can delete messages it doesn't need any more
alias wtcleos='cleos -v -u https://testnet.waxsweden.org'
wtcleos push action poboxxxxxxxx accept '["poboxrcpnt11", 2, 100]' -p poboxrcpnt11@active
```

## The method

ECDH allows deriving a shared secret between two parties, by group
multiplication of Alice's private and Bob's public keys. Then Bob can
do group multiplication of his private key and Alice's public key, and
derive the same shared secret and use it for decryption.

If Alice uses the same private key, the shared secret will be same, so
there's a risk of compromising all the messages.

To circumvent that, Alice generates a random ephemeral key every time
she needs to send a message to Bob. Bob is able to decrypt the
messages knowing the ephemeral public key and using his private key.

Alice sends the message using the public blockchain, so the
transaction is signed by her, and the smart contract verifies her
authorization.

Each message is accompanied by a MAC, so that only Bob's private key
is able to produce a matching MAC. This prevents Bob from using a
wrong key for decryption.


## Multiple recipients

Say, Alice needs to send a piece of secret data to Bob and Chris.

ECDH algorithm does not allow multiple recipients: only one public key
can be used to derive the shared secret. There's a method of having
multiple senders, but they all need to apply their private keys, and
the whole ceremony becomes complex with more than two senders.

So, in order to share the secret information with multiple recipients,
an additional layer of communication is required.

For example, Alice sends secret messages to Bob and Chris, and the
messages contain the location of the file (such as IPFS hash, or some
other storage), and encryption password.

If Alice wants to share the secret data with more parties (Diana,
Elvis), she can send an encrypted message to herself before sending to
Bob and Chris. So, when adding a new party, she decrypts the data
location and password, and encrypts new messages to Diana and Elvis.







## Copyright and License

Copyright 2020 cc32d9@gmail.com

This work is licensed under a Creative Commons Attribution 4.0
International License.

http://creativecommons.org/licenses/by/4.0/
