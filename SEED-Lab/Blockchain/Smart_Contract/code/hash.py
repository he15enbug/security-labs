#!/usr/bin/env python3
from web3 import Web3

# func_sig = "increaseCounter(uint256)"
func_sig = "decreaseCounter(uint256)"

hash = Web3.sha3(text=func_sig)
print(hash.hex())
