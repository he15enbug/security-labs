#!/bin/bash

rm -f *.txt
rm -f *.bin

########## Generate Plaintext ##########
for i in {0..100}
do
    echo -n "0123456789" >> plain.txt
done

MODES=("ecb" "cbc" "cfb" "ofb")

for i in {0..3}
do
    K_IV="-K 00112233445566778889aabbccddeeff -iv 0102030405060708"
    MODE=${MODES[$i]}

    # ECB will not require a IV
    if [ $i -eq 0 ]; then
        K_IV="-K 00112233445566778889aabbccddeeff"
    fi

    # Encrypt
    echo "Encrypt"
    openssl enc -aes-128-$MODE -e -in plain.txt -out "$MODE".bin $K_IV

    # Corrupt the 55th byte
    echo "Corrupt the 55th byte"

    head -c 54 "$MODE".bin > pre.tmp
    printf "\x88" > byte55.tmp
    tail -c +56 "$MODE".bin > suf.tmp
    cat pre.tmp byte55.tmp suf.tmp > "$MODE"_modified.bin
    rm *.tmp

    # Decrypt
    echo "Decrypt"
    openssl enc -aes-128-$MODE -d -in "$MODE"_modified.bin -out "$MODE"_dec.txt $K_IV
done
