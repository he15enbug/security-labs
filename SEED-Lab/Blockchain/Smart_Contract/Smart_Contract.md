# Smart Contract Lab
- a smart contract is a program that runs on a blockchain
- its code, which is immutable and stored at a particular address of the blockchain, gets executed automatically when a predetermined condition is met
- its data is also stored on the blockchain
- topics
    - Smart contract development
    - Remix IDE
    - Deploy and interact with smart contract
    - Send fund to and from smart contract

## Lab Setup
- use `Labsetup/emulator_10`

## Task 1: Using Remix for Smart Contract Development
- there are several development environments for smart contracts, including Hardhat, Remix, and Truffle. [Remix IDE](https://remix.ethereum.org) is used in this lab. Here are the [instructions for Remix IDE](https://remix-ide.readthedocs.io)
- Remix can connect to blockchain via several mechanisms, such as Remix VM, injected provider (MetaMask), and external HTTP providers. The Remix VM is a sandbox blockchain in the browser, it simulates the behavior of a blockchain
- we will connect Remix to the SEED emulator, which is a real private blockchain
- steps
    1. after starting the docker containers, set up MetaMask, join the SEED emulator
    2. in Remix IDE, in the `Deploy & Run Transactions` menu, set `Environment` to `"Injected Provider - MetaMask"`
    3. In the `Account` drop-down menu, we can see several accounts with balance

### Task 1.b: Write, Compile, and Deploy Smart Contract
- write a simple smart contract `Hello.sol`
- compile: go to the `Solidity compiler` menu, and compile the program
- deploy: go to the `Deploy & Run Transactions` menu, click `Deploy` button. MetaMask will come up and ask for confirmation
- in the `Deploy & Run Transactions` menu, check `Deployed Contracts`, the address of this smart contract is `0xd5900db429fab4154B07eb384F7509cf0cc330aB`
### Task 1.c: Under the hood
- deploying a contract is done through a transaction, which is different from fund-transfer transaction
- we can see the transaction details in EtherView (http://localhost:5000)
- the recepient of this transaction is `null`
- in the data field, the content is `0x6080...0033`. In the `Deploy & Run Transactions` menu, we can save transactions recorded as a `json` file, then we can see the following content, and `0x6080...0033` is actually the bytecode of the smart contract
    ```
    ...
    {
        "timestamp": 1706947089897,
        "record": {
            "value": "0",
            "inputs": "()",
            "parameters": [],
            "name": "",
            "type": "constructor",
            "abi": "0x3c94...5c80",
            "contractName": "Hello",
            "bytecode": "6080...0033",
            "linkReferences": {},
            "from": "account{0}"
        }
    }
    ...
    ```

## Task 2: Invoke Contract Functions
- In the `Deployed Contracts` region, we can see the functions of deployed contracts
### Task 2.a: Invoke a function via local call
- if a function is defined as `public view` type, it does not modify the state of the contract, and is thus invoked through a local call, instead of via a transaction. There is no gas cost for this type of interaction
- Remix creates a button for this type of function, as well as for the `public` variable (for each `public` variable, a default getter of the `public view` type is created)
- click the `getResult` (provide 2 arguments), `owner`, and `sayHello` button, we can see the result immediately, because there is no transactions sent

### Task 2.b: Invoke a function via transaction
- if a function modified the state of the contract, it has to be invoked via a transaction
- invoke `increaseCounter()`, provide the argument, then confirm on MetaMask, if we open EtherView, we can see a new transaction (`{from: caller address, to: contract address}`)
- add a new function `decreaseCounter(uint k)`, compile, deploy and call this function, a message will be displayed showing that this call is likely to fail, the reason is that `counter` is a `256-bit unsigned int` with a default value `0`, if we decrease it, there will be an underflow
    - call `increaseCounter(100)` first, then call `decreaseCounter(10)`, this will succeed

### Task 2.c: Under the hood
- after calling `increaseCounter(100)`, in the transaction:
    - `to`: the address of the contract
    - `data` (`4 + 32` bytes): `0x9e80c0740000000000000000000000000000000000000000000000000000000000000064`

- the data field consists of a function selector (the first `4` bytes of the hash of the function's prototype), and the parameter (`32` bytes)
    - we can use the following program to get the hash (`sha3`) of `increaseCounter(uint256)`, its `0x9e80c074933ed616d86c79d926a1dfd97afbe0ba20c42d692de450d20df14d77`
        ```
        Web3.sha3(text='increaseCounter(uint256)').hex()
        ```
- instead of clicking the button, construct the data and put it in the data field to invoke a function, e.g., `decreaseCounter(20)`
    - construct the data
        1. first 4 bytes of `0xb24f03afec3fac4b94910b75cb3164e40c812facae5e62387d05cb3a8e4584a0`
        2. parameter `0x0000000000000000000000000000000000000000000000000000000000000014`
    - put `0xb24f03af0000000000000000000000000000000000000000000000000000000000000014` in `CALLDATA` field, and click `Transact`

- **problem**: when I click `Transact`, I got a red hint: `'Fallback' function is not defined`, we need to add a function `fallback() external payable { }`, and compile, deploy, then invoke functions

### Task 2.d: Emit events
- message generated by `emit` will be placed inside the log field of a transaction receipt
- for `emit MyMessage(msg.sender, counter)`, the result is in `args` field of the log
    ```
    "args": {
        "0": <address of the caller>,
        "1": <value of counter>,
        "_from": <address of the caller>,
        "_value": <value of counter>
    }
    ```
## Task 3: Send Fund to Contract
- use `EtherFaucet.sol`, when we send fund to a smart contract, some of the contract's function (must be `payable` type) will be invoked, the logic:
    - Ether received, is `msg.data` empty (i.e., no function is specified)?
        - yes, `receive()` exists?
            - yes, invoke `receive()`
            - no, invoke `fallback()`
        - no, is specified function exists?
            - yes, invoke the specified function
            - no, invoke `fallback()`

### Task 3.a: Send fund directly to a contract address
- deploy `EtherFaucet`, the address is `0xa35dF634F3FA566DD2F29b36D1961663342b1eD3`
- use MetaMask to send Ether to this address
- in `Deployed Contracts`, we can see the balance of the contract is `1 ETH`

### Task 3.b: Send fund to a payable function
- invoke `donateEther()`, input the amount in the `value` field (send `2 ETH`)
- the balance of the contract is `3 ETH`
- result, from the value of `amount` we can know that sending Ether to a contract by calling its function will not trigger `receive()` or `fallback()`, otherwise, the amount should be `5000000000000000000`
    - `amount`: `3000000000000000000` (`Wei`)
    - `donationCount`: `1`

### Task 3.c: Send fund to a non-payable function
- if we invoke `donateEtherWrong()`, which is not a `payable` function, the transaction will fail
- `donationCount` is still `1`, the sender won't lose money

### Task 3.d: Send fund to a non-existing function
- invoke a function called `foo()`, which is not exist, the first 4 bytes of `sha3(text="foo()")` is `0xc2985578`, it has no argument, so the calldata is `0xc2985578`
- the value is `1 ETH`
- the transaction is confirmed, the Ether is successfully sent to the contract
- `fallbackCount` is `1`, `receiveCount` is `1` (`receive()` was invoked before when we send money to the contract using MetaMask), so the function that is actually invoked is `fallback()`

## Task 4: Send Fund from Contract
- 3 methods for sending ether from a contract, `send`, `transfer`, `call`, all of them are translated into the `CALL` opcode by Solidity compiler, so they share the same internal process, but they have differences
- the major difference is the gas limit forwarded to the recipient. When a transaction is sent, a gas limit is set by the original sender. When this transaction invokes a contract A, which triggers the contract to send ether to a recipient, if the recipient is also a contract (B), some of B's code will be executed. B needs some gas to run, and how much gas it gets depends on how much is forwarded by the sender contract A
    - `send` and `transfer` methods set a limit of fixed `2300` gas forwarded to the recipient, with this limited gas, B cannot do much, this is to prevent the reentrancy attack
    - `call` by default doesn't have such a limit, i.e., it forwards all the gas set by the original sender, we can set a limit manually when using `call`

- Task: Send fund to an external owned account
    - use `sendEtherViaSend`, `sendEtherViaCall`, `sendEtherViaTransfer`
    - all of them works

## Task 5: Invoke Another Contract
- use `Caller.sol` to invoke `Hello.sol`,  it needs to include an `interface` derived from the `Hello` contract
- deploy `Hello` (at `0x600650Df7a3a4c66b21eaeF1e8eF0a6931aBc24b`)
- deploy `Caller` (at `0x8ae2B877e6222632090B44EaEDC1eC3A6c85008B`)
- invoke `invokeHello("0x600650Df7a3a4c66b21eaeF1e8eF0a6931aBc24b", 10)` in `Caller`
    - succeed
    - in `Hello`, invoke `getCounter`: `10`
- invoke `invokeHello2("0x600650Df7a3a4c66b21eaeF1e8eF0a6931aBc24b", 2)` in `Caller`
    - before invoking, ensure the balance of `Caller` is greater than `1 ETH` (I sent `3 ETH`, and the balance of `Hello` is `0`)
    - succeed
    - in `Hello`, invoke `getCounter`: `12`
    - balance of `Hello`: `1 ETH`
