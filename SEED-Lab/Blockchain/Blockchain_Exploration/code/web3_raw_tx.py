#!/bin/env python3
from web3 import Web3
from eth_account import Account

# Connect to any of the nodes in the blockchain network
web3 = Web3(Web3.HTTPProvider('http://10.154.0.71:8545'))

# Sender's private key 
key = 'e128a6b87aa1d934970fd0f2714dd2fe61c017636725dbfeb5e487cc83bcb7eb'
sender = Account.from_key(key)

recipient = Web3.toChecksumAddress('0xF5406927254d2dA7F7c28A61191e3Ff1f2400fe9')
tx = {
  'chainId': 1337, 
  'nonce'  : web3.eth.getTransactionCount(sender.address),
  'from'   : sender.address,
  'to'     : recipient,
  'value'  : Web3.toWei("5", 'ether'),
  'gas'    : 200000,
  'maxFeePerGas'        : Web3.toWei('4', 'gwei'),
  'maxPriorityFeePerGas': Web3.toWei('3', 'gwei'),
  'data'                : ''
}

# Sign the transaction and send it out
signed_tx  = web3.eth.account.sign_transaction(tx, sender.key)
tx_hash    = web3.eth.sendRawTransaction(signed_tx.rawTransaction)

# Wait for the transaction to appear on the blockchain
print("Transaction sent, waiting for receipt ...")
tx_receipt = web3.eth.wait_for_transaction_receipt(tx_hash)
print("Transaction Receipt: {}".format(tx_receipt))
