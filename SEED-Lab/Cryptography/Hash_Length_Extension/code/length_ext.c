#include <stdio.h>
#include <arpa/inet.h>
#include <openssl/sha.h>

int main(int argc, const char *argv[]) {
    int i;
    unsigned char buffer[SHA256_DIGEST_LENGTH];
    SHA256_CTX c;

    SHA256_Init(&c);
    for(i = 0; i < 64; i++) {
        SHA256_Update(&c, "*", 1);
    }

    // MAC of the original message M (padded)
    c.h[0] = htole32(0xf45212a2);
    c.h[1] = htole32(0xc35ae176);
    c.h[2] = htole32(0x87d00d0b);
    c.h[3] = htole32(0xd4f0c248);
    c.h[4] = htole32(0xeaaabb18);
    c.h[5] = htole32(0x93399da1);
    c.h[6] = htole32(0xa12683b2);
    c.h[7] = htole32(0x9779b7ed);

    // Append additional message
    SHA256_Update(&c, "&download=secret.txt", 20);
    SHA256_Final(buffer, &c);

    for(i = 0; i < 32; i++) {
        printf("%02x", buffer[i]);
    }
    printf("\n");
    return 0;
}