# Blockchain Exploration Lab
- get familiar with blockchain, in particular, the Ethereum blockchain
- topics
    - MetaMask, wallet, account
    - Transactions and blocks, sending transactions
    - Ethereum nodes

## Lab Setup
- use `emulator_10`, it is for AMD64 machines, and there are 10 nodes on the blockchain network
- for sake of simplicity, the blockchain running inside the emulator uses the Proof-of-Authority (PoA) consensus protocol, instead of the Proof-of-Stack protocol used in the Ethereum Mainnet. The activities conducted in this lab are not dependent on any specific consensus protocol
- **EtherView**, a simple web application (`http://localhost:5000`) that displays the activities on the Blockchain

## Task 1: Setting Up MetaMask Wallet
- for the basic interactions with blockchain, we can use a wallet application, which manages our keys, and displays some basic informations
### Task 1.a: Installing the MetaMask extension
- I installed MetaMask on both Firefox and Chrome, however, on Firefox, it stucks at the loading page, so I used MetaMask on Chrome
### Task 1.b: Connecting to the Blockchain
- add a network manually, connect to `10.154.0.71` or any one of the nodes
- network information
    ```
    name:            SEED emulator
    New RPC URL:     http://10.154.0.71:8545
    Chain ID:        1337
    Currency symbol: ETH
    ```
### Task 1.c: Adding accounts
- use the mnemonic phrase provided: `gentle always fun glass foster produce north tail security list example gain`
- the address of these account are:
    - `0xF5406927254d2dA7F7c28A61191e3Ff1f2400fe9` (balance: `30 ETH`)
    - `0x2e2e3a61daC1A2056d9304F79C168cD16aAa88e9` (balance: `9999999 ETH`)
    - `0xCBF1e330F0abD5c1ac979CF2B2B874cfD4902E24` (balance: `10 ETH`)

### Task 1.d: Connecting to the Blockchain
- send `100 ETH` from `0x2e2e3a61daC1A2056d9304F79C168cD16aAa88e9` to `0xCBF1e330F0abD5c1ac979CF2B2B874cfD4902E24`
- total gas fee is about `0.000031 ETH`

## Task 2: Interacting with Blockchain Using Python
### Task 2.a: Install Python modules
- use `web3` and `docker` modules: `pip3 install web3==5.31.1 docker`
### Task 2.b: Checking account balance
- `web3_balance.py`
    ```
    #!/bin/env python3
    from web3 import Web3

    # Connect to a node in the blockchain network
    url  = 'http://10.154.0.71:8545'
    web3 = Web3(Web3.HTTPProvider(url)) 

    addr = Web3.toChecksumAddress('')
    balance = web3.eth.get_balance(addr) # Get the balance
    print(addr + ": " + str(Web3.fromWei(balance, 'ether')) + " ETH")
    ```
- check the balance of these three accounts
    ```
    $ ./web3_balance.py 
    0xF5406927254d2dA7F7c28A61191e3Ff1f2400fe9: 30 ETH
    0x2e2e3a61daC1A2056d9304F79C168cD16aAa88e9: 9999898.999968499999853 ETH
    0xCBF1e330F0abD5c1ac979CF2B2B874cfD4902E24: 110 ETH
    ```
### Task 2.c: Sending Transactions
- send `5 ETH` from `0xF5406927254d2dA7F7c28A61191e3Ff1f2400fe9` to `0xF5406927254d2dA7F7c28A61191e3Ff1f2400fe9`
- `web3_raw_tx.py`
    ```
    from web3 import Web3
    from eth_account import Account

    # Connect to a node in the blockchain network
    web3 = Web3(Web3.HTTPProvider('http://10.154.0.71:8545')) 
    
    # Sender's private key
    key    = 'e128a6b87aa1d934970fd0f2714dd2fe61c017636725dbfeb5e487cc83bcb7eb' <---- put the private key here
    sender = Account.from_key(key)

    recipient = Web3.toChecksumAddress('0xF5406927254d2dA7F7c28A61191e3Ff1f2400fe9')

    tx = {
        'chainId': 1337,
        'nonce'  : web3.eth.getTransactionCount(sender.address),
        'from'   : sender,
        'to'     : recipient,
        'value'  : Web3.toWei('5', 'ether'),
        'gas'    : 200000,
        'maxFeePerGas'         : Web3.toWei('4', 'gwei'),
        'maxPriorityFeePerGas' : Web3.toWei('3', 'gwei'),
        'data'                 : ''
    }

    # Sign the tx and send it out
    signed_tx  = web3.eth.account.sign_transaction(tx, sender.key)
    tx_hash    = web3.eth.sendRawTransaction(signed_tx.rawTransaction)

    # Wait for the tx to appear on the blockchain
    tx_receipt = web3.eth.wait_for_transaction_receipt(tx_hash)
    print("Transaction Receipt: {}".format(tx_receipt))
    ```
- test
    ```
    $ ./web3_raw_tx.py 
    Transaction sent, waiting for receipt ...
    Transaction Receipt: AttributeDict({'blockHash': HexBytes('0x699c623358112491e2dfdcccda637eb04f01b3e6ef4d16dacc0a8686c729754c'), 'blockNumber': 4907, 'contractAddress': None, 'cumulativeGasUsed': 21000, 'effectiveGasPrice': 3000000007, 'from': '0xCBF1e330F0abD5c1ac979CF2B2B874cfD4902E24', 'gasUsed': 21000, 'logs': [], 'logsBloom': HexBytes('0x00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000'), 'status': 1, 'to': '0xF5406927254d2dA7F7c28A61191e3Ff1f2400fe9', 'transactionHash': HexBytes('0x12382aaa1ee4b6c74e280e5d13956c5762955fc9d3a26e18c350871b51d3d34b'), 'transactionIndex': 0, 'type': '0x2'})
    $ ./web3_balance.py 
    0xF5406927254d2dA7F7c28A61191e3Ff1f2400fe9: 35 ETH
    0x2e2e3a61daC1A2056d9304F79C168cD16aAa88e9: 9999898.999968499999853 ETH
    0xCBF1e330F0abD5c1ac979CF2B2B874cfD4902E24: 104.999936999999853 ETH
    ```

## Task 3: Interacting with Blockchain Using Geth
- interact with the blockchain directly from a blockchain node
- in the emulator, each node runs the Geth (go-ethereum), which is a Go implementation of Ethereum
- there are many ways to interact with the Geth client, including `HTTP`, `websockets`, and `IPC`. When we interact with the Geth nodes using MetaMask or Python, we are using the JSON-RPC method, we can also log into a Geth node, and communicate with it using local IPC, the following `geth` command gets an interactive console on the node:
    ```
    # geth attach
    Welcome to the Geth JavaScript console!
    ...
    To exit, press ctrl-d
    >
    ```
- it is an interactive JavaScript console, so we can run JS code in it, the `eth` class has many APIs to interact with the blockchain, e.g., get the balance of an account
    ```
    > myaccount = "0xF5406927254d2dA7F7c28A61191e3Ff1f2400fe9"
    "0xF5406927254d2dA7F7c28A61191e3Ff1f2400fe9"
    > eth.getBalance(myaccount)
    35000000000000000000
    ```

### Task 3.a: Getting balance
- the balance of the other 2 accounts
    ```
    > eth.getBalance("0x2e2e3a61daC1A2056d9304F79C168cD16aAa88e9")
    9.999898999968499999853e+24
    > eth.getBalance("0xCBF1e330F0abD5c1ac979CF2B2B874cfD4902E24")
    104999936999999853000
    ```

### Task 3.b: Sending transactions
- each node maintains a list of accounts, stored in `/root/.ethereum/keystore`, their addresses are loaded into the `eth.accounts[]` array, e.g., to get the first account address, use `eth.accounts[0]`
- these accounts are locked (encrypted) using a password, to use them to send transactions, we need to decrypt them using the password. In these emulators, all accounts are encrypted using password `admin`
- unlock account and send a transaction
    ```
    > eth.accounts
    ["0x72943017a1fa5f255fc0f06625aec22319fcd5b3"]
    > personal.unlockAccount(eth.accounts[0])
    Unlock account 0x72943017a1fa5f255fc0f06625aec22319fcd5b3
    Passphrase: 
    true
    > sender = eth.accounts[0]
    "0x72943017a1fa5f255fc0f06625aec22319fcd5b3"
    > target = "0xF5406927254d2dA7F7c28A61191e3Ff1f2400fe9"
    "0xF5406927254d2dA7F7c28A61191e3Ff1f2400fe9"
    > amount = web3.toWei(0.2, "ether")
    "200000000000000000"
    > eth.sendTransaction({from: sender, to: target, value: amount})
    "0xafb187ae664a3120325ddcdd7e4862321c586b41bbcd3b560cea52eb0581aab9"
    > eth.getBalance("0xF5406927254d2dA7F7c28A61191e3Ff1f2400fe9")
    35200000000000000000
    ```
- if the sender account is locked, sending transaction will cause an error
    ```
    > personal.lockAccount(sender)
    true
    > amount = web3.toWei(10, "ether")
    "10000000000000000000"
    > eth.sendTransaction({from: sender, to: target, value: amount})
    Error: authentication needed: password or unlock
        at web3.js:6365:9(45)
        at send (web3.js:5099:62(34))
        at <eval>:1:20(10)
    ```

### Task 3.c: Sending transactions from a different account
- if the sender is not in `eth.accounts[]`, there is an error (unknown account), the reason is that sending transaction requires the private key of the sender, if the private key is not stored on this node, there will be an error
    ```
    > sender = "0xCBF1e330F0abD5c1ac979CF2B2B874cfD4902E24"
    "0xCBF1e330F0abD5c1ac979CF2B2B874cfD4902E24"
    > target = "0xF5406927254d2dA7F7c28A61191e3Ff1f2400fe9"
    "0xF5406927254d2dA7F7c28A61191e3Ff1f2400fe9"
    > amount = web3.toWei(7, "ether")
    "7000000000000000000"
    > eth.sendTransaction({from: sender, to: target, value: amount})
    Error: unknown account
        at web3.js:6365:9(45)
        at send (web3.js:5099:62(34))
        at <eval>:1:20(10)
    ```

## Task 4: Adding a Full Node
- join an existing blockchain network
- use the empty container, with `new_eth_node` encoded in its name
- steps
    1. initialize the node using blockchain information, which is captured in its genesis block (the first block of a block chain). Get a copy of the genesis block from an existing Ethereum node (in `/tmp/eth-genesis.json`), put it in the new node's root folder `/`
        ```
        root@4fd34cd3e929 / # geth --datadir /root/.ethereum init /eth-genesis.json 
        INFO [02-02|14:39:04.121] Maximum peer count                       ETH=50 LES=0 total=50
        ...
        INFO [02-02|14:39:04.474] Writing custom genesis block 
        INFO [02-02|14:39:04.475] Persisted trie from memory database      nodes=33 size=4.78KiB time="217.316µs" gcnodes=0 gcsize=0.00B gctime=0s livenodes=1 livesize=0.00B
        INFO [02-02|14:39:04.476] Successfully wrote genesis state         database=lightchaindata                      hash=54f1a3..3303ab
        ```
    2. run `geth` coomand to join an existing blockchain network
        - provide a list of bootnodes, go to any of existing nodes (non-bootnode), we will see a file called `eth-node-urls` in `/tmp`, it contains a list of the bootnode, copy and paste it's content to the new node

        - feed the content of `/tmp/eth-node-urls` to the `bootnodes` option
            ```
            # geth --datadir /root/.ethereum \
            --identity="NEW_NODE_01" --networkid=1337 \
            --syncmode full --snapshot=False --verbosity=2 --port 30303 \
            --bootnodes "$(cat /tmp/eth-node-urls)" \
            --allow-insecure-unlock \
            --http --http.addr 0.0.0.0 --http.corsdomain "*" \
            --http.api web3,eth,debug,personal,net,clique,engine,admin,txpool
            ```
- run `geth attach`, we can enter the JS console
    ```
    # geth attach
    ...
    > 
    > admin.peers
    [{
        caps: ["eth/66", "eth/67"],
        enode: ...,
        id: "5e7765133d73e9c784f261d0cc6a37ad5535e5c77d4aba49afea0ad40cb05e73",
        name: "Geth/NODE_3/v1.10.26-stable-e5eb32ac/linux-amd64/go1.18.10",
        network: {
            inbound: false,
            localAddress: "10.150.0.74:38672",
            remoteAddress: "10.152.0.71:30303",
            static: false,
            trusted: false
        },
        protocols: {
            eth: {
                difficulty: 10475,
                head: "0xd54267c8aef9abca7dc3f88ed306f5ec6477a443f5f7a5b7f1580b084bd3e89c",
                version: 67
            }
        }
    }, 
    ...
    ] <---- the information of a list of other nodes in the blockchain network
    ```
- create a new account using `personal.newAccount()`
    ```
    > personal.newAccount()
    Passphrase: 
    Repeat passphrase: 
    "0x519cd597f64c25d45d9110560805154b5acbb506"
    > eth.accounts
    ["0x519cd597f64c25d45d9110560805154b5acbb506"]
    ```

- modify the Python code `web3_raw_tx.py`
    - in previous steps, we didn't set the `http.port`, by default, it is `8545`, the ip address of the new node is `10.150.0.74`
    - connect to this new node `Web3(Web3.HTTPProvider("10.150.0.74:8545"))`
    - run it
        ```
        $ ./web3_raw_tx_new.py 
        Transaction sent, waiting for receipt ...
        Transaction Receipt: ...
        ```
    - check the balance    
        ```
        > eth.getBalance(eth.accounts[0])
        5000000000000000000
        ```

- send a transaction in the JS console
    ```
    > personal.unlockAccount(eth.accounts[0])
    Unlock account 0x519cd597f64c25d45d9110560805154b5acbb506
    Passphrase: 
    true
    > sender=eth.accounts[0]
    "0x519cd597f64c25d45d9110560805154b5acbb506"
    > target="0xF5406927254d2dA7F7c28A61191e3Ff1f2400fe9"
    "0xF5406927254d2dA7F7c28A61191e3Ff1f2400fe9"
    > amount=web3.toWei(1.5,'ether')
    "1500000000000000000"
    > eth.sendTransaction({from:sender,to:target,value:amount})
    "0xc3fe702c0ae427b94a87e0587aa910789b92a82739f2f45e049cf97449519ae0"
    > eth.getBalance(eth.accounts[0])
    3499978999999853000  <---- balance deducted
    ```
