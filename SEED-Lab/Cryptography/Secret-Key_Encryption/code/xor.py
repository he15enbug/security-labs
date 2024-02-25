#!/usr/bin/python3

# XOR two bytearrays
def xor(first, second):
    return bytearray(x^y for x,y in zip(first, second))

MSG   = "This is a known message!"
HEX_1 = "a469b1c502c1cab966965e50425438e1bb1b5f9037a4c159"
HEX_2 = "bf73bcd3509299d566c35b5d450337e1bb175f903fafc159"

# Convert ascii string to bytearray
D1 = bytes(MSG, 'utf-8')

# Convert hex string to bytearray
D2 = bytearray.fromhex(HEX_1)
D3 = bytearray.fromhex(HEX_2)

# Task 6.2
# tmp = xor(D1, D2)
# P2  = xor(tmp, D3)
# print(P2.hex())
# print(P2.decode('utf-8'))

# Task 6.3
YES = bytearray.fromhex("5965730D0D0D0D0D0D0D0D0D0D0D0D0D")
NO  = bytearray.fromhex("4e6f0E0E0E0E0E0E0E0E0E0E0E0E0E0E")
IV0 = bytearray.fromhex("1700d2a48d1e1138da3112d44fda20a6")

YES_XOR_IV0 = xor(YES, IV0)
NO_XOR_IV0  = xor(NO , IV0)

print(YES_XOR_IV0.hex())
print( NO_XOR_IV0.hex())

IV1 = bytearray.fromhex("e9d563218e1e1138da3112d44fda20a6")
P1  = xor(IV1, YES_XOR_IV0)
print(P1.hex())
# IV2 = bytearray.fromhex("431c545e8e1e1138da3112d44fda20a6")
# print(xor(IV2, NO_XOR_IV0).hex())
