#!/bin/env python3
from web3 import Web3

url  = 'http://10.154.0.71:8545'
web3 = Web3(Web3.HTTPProvider(url))  # Connect to a node in the blockchain network

addr = Web3.toChecksumAddress('0xF5406927254d2dA7F7c28A61191e3Ff1f2400fe9')
balance = web3.eth.get_balance(addr) # Get the balance 
print(addr + ": " + str(Web3.fromWei(balance, 'ether')) + " ETH")

addr = Web3.toChecksumAddress('0x2e2e3a61daC1A2056d9304F79C168cD16aAa88e9')
balance = web3.eth.get_balance(addr) # Get the balance 
print(addr + ": " + str(Web3.fromWei(balance, 'ether')) + " ETH")

addr = Web3.toChecksumAddress('0xCBF1e330F0abD5c1ac979CF2B2B874cfD4902E24')
balance = web3.eth.get_balance(addr) # Get the balance 
print(addr + ": " + str(Web3.fromWei(balance, 'ether')) + " ETH")
