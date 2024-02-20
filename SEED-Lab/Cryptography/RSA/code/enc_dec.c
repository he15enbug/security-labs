#include <stdio.h>
#include <openssl/bn.h>

void printBN(char *msg, BIGNUM * a) {
    char * number_str = BN_bn2hex(a);
    printf("%s %s\n", msg, number_str);
    OPENSSL_free(number_str);
}

BIGNUM *enc(BIGNUM *plaintext, BIGNUM *e, BIGNUM *n, BN_CTX *ctx) {
    BIGNUM *ciphertext = BN_new();
    BN_mod_exp(ciphertext, plaintext, e, n, ctx);
    return ciphertext;
}

BIGNUM *dec(BIGNUM *ciphertext, BIGNUM *d, BIGNUM *n, BN_CTX *ctx) {
    BIGNUM *plaintext = BN_new();
    BN_mod_exp(plaintext, ciphertext, d, n, ctx);
    return plaintext;
}

int main () {
    BN_CTX *ctx = BN_CTX_new();

    BIGNUM *msg = BN_new();
    BN_hex2bn(&msg, "4120746f702073656372657421");
    BIGNUM *ciphertext = BN_new();
    BN_hex2bn(&ciphertext, "8C0F971DF2F3672B28811407E2DABBE1DA0FEBBBDFC7DCB67396567EA1E2493F");

    BIGNUM *e = BN_new();
    BIGNUM *n = BN_new();
    BIGNUM *d = BN_new();
    BN_hex2bn(&n, "DCBFFE3E51F62E09CE7032E2677A78946A849DC4CDDE3A4D0CB81629242FB1A5");
    BN_hex2bn(&e, "010001");
    BN_hex2bn(&d, "74D806F9F3A62BAE331FFE3F0A68AFE35B3D2E4794148AACBC26AA381CD7D30D");

    // task 2 encrypt hex string 4120746f702073656372657421
    printBN("ciphertext for \"4120746f702073656372657421\":", enc(msg, e, n, ctx));

    // task 3 decrypt ciphertext 8C0F971DF2F3672B28811407E2DABBE1DA0FEBBBDFC7DCB67396567EA1E2493F
    printBN("plaintext for \"8C0F971DF2F3672B28811407E2DABBE1DA0FEBBBDFC7DCB67396567EA1E2493F\":", dec(ciphertext, d, n, ctx));

    return 0;
}