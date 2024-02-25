#!/usr/bin/python3

import socket
from binascii import hexlify, unhexlify

# XOR two bytearrays
def xor(first, second):
   return bytearray(x^y for x,y in zip(first, second))

class PaddingOracle:

    def __init__(self, host, port) -> None:
        self.s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.s.connect((host, port))

        ciphertext = self.s.recv(4096).decode().strip()
        self.ctext = unhexlify(ciphertext)

    def decrypt(self, ctext: bytes) -> None:
        self._send(hexlify(ctext))
        return self._recv()

    def _recv(self):
        resp = self.s.recv(4096).decode().strip()
        return resp 

    def _send(self, hexstr: bytes):
        self.s.send(hexstr + b'\n')

    def __del__(self):
        self.s.close()

def get_block(IV, C, C_NEXT, oracle):
    P  = bytearray(16)
    D  = bytearray(16)
    CC = bytearray(16)
    # Any value is ok
    for i in range(0, 16):
        D[i]  = 0x00
        CC[i] = 0x77
        
    for K in range(1, 17):
        pad_byte = K
        for i in range(256):
            CC[16 - K] = i
            status = oracle.decrypt(IV + CC + C_NEXT)
            if status == "Valid":
                print("Valid: i = 0x{:02x}".format(i))
                print("CC: " + CC.hex())
                break
        # compute D[16 - K] and P[16 - K]
        D[16 - K] = pad_byte  ^ CC[16 - K]
        P[16 - K] = D[16 - K] ^  C[16 - K]
        print("D: i = 0x{:02x}".format(D[16 - K]))
        print("P: i = 0x{:02x}".format(P[16 - K]))
        
        # adjust CC to make padding[16-K] to padding[15] become (K+1)
        for i in range(16 - K, 16):
            CC[i] = D[i] ^ (K+1)
    print("P: " + P.hex())
    return P

if __name__ == "__main__":
    oracle = PaddingOracle('10.9.0.80', 6000)

    # Get the IV + Ciphertext from the oracle
    iv_and_ctext = bytearray(oracle.ctext)
    IV    = iv_and_ctext[00:16]
    C1    = iv_and_ctext[16:32]  # 1st block of ciphertext
    C2    = iv_and_ctext[32:48]  # 2nd block of ciphertext
    C3    = iv_and_ctext[48:64]  # 3rd block of ciphertext

    print("C1:  " + C1.hex())
    print("C2:  " + C2.hex())
    print("C3:  " + C3.hex())

    # Launch the padding oracle attack automatically
    P3 = get_block(IV, C2, C3, oracle)
    P2 = get_block(IV, C1, C2, oracle)
    P1 = get_block(IV, IV, C1, oracle)
    
    print("The Plaintext (Padded):")
    print(P1.hex() + P2.hex() + P3.hex())
