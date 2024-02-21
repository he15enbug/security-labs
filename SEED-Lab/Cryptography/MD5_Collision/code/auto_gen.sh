#!/bin/bash

gcc -o xyz xyz.c

head -c 12320 xyz > prefix
md5collgen -p prefix -o pre1 pre2

tail -c 160 pre1 > b1
tail -c 4480 xyz > rest
cat b1 rest > suffix

head -c 12544 xyz > temp
tail -c 64 temp > middle
cat pre1 middle suffix > benign
cat pre2 middle suffix > malicious

# clear temporary files
rm temp
rm b1
rm rest
rm prefix
rm pre1
rm pre2
rm middle
rm suffix

# verify MD5
echo "check MD5..."
md5sum benign
md5sum malicious

# verify the programs' behavior
chmod 777 benign
chmod 777 malicious
echo "run the benign program..."
./benign
echo "run the malicious program..."
./malicious