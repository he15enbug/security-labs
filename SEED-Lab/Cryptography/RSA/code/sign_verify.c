#include <stdio.h>
#include <openssl/bn.h>
#include <stdbool.h>

void printBN(char *msg, BIGNUM * a) {
    char * number_str = BN_bn2hex(a);
    printf("%s %s\n", msg, number_str);
    OPENSSL_free(number_str);
}

BIGNUM *sign(BIGNUM *message, BIGNUM *d, BIGNUM *n, BN_CTX *ctx) {
    BIGNUM *signature = BN_new();
    BN_mod_exp(signature, message, d, n, ctx);
    return signature;
}

BIGNUM *sign_hex(char *message_hex, BIGNUM *d, BIGNUM *n, BN_CTX *ctx) {
    BIGNUM *message = BN_new();
    BN_hex2bn(&message, message_hex);

    BIGNUM *signature = BN_new();
    BN_mod_exp(signature, message, d, n, ctx);
    return signature;
}

bool verify(BIGNUM *message, BIGNUM *signature, BIGNUM *e, BIGNUM *n, BN_CTX *ctx) {
    BIGNUM *msg = BN_new();
    BN_mod_exp(msg, signature, e, n, ctx);
    printBN("original message being signed:", msg);
    return (BN_cmp(message, msg) == 0);
}

bool verify_hex(char *message_hex, char *signature_hex, BIGNUM *e, BIGNUM *n, BN_CTX *ctx) {
    BIGNUM *message = BN_new();
    BN_hex2bn(&message, message_hex);
    BIGNUM *signature = BN_new();
    BN_hex2bn(&signature, signature_hex);

    BIGNUM *msg = BN_new();
    BN_mod_exp(msg, signature, e, n, ctx);
    printBN("original message being signed:", msg);
    return (BN_cmp(message, msg) == 0);
}

int main () {
    BN_CTX *ctx = BN_CTX_new(); 

    BIGNUM *e = BN_new();
    BIGNUM *n = BN_new();
    BIGNUM *d = BN_new();
    BN_hex2bn(&n, "DCBFFE3E51F62E09CE7032E2677A78946A849DC4CDDE3A4D0CB81629242FB1A5");
    BN_hex2bn(&e, "010001");
    BN_hex2bn(&d, "74D806F9F3A62BAE331FFE3F0A68AFE35B3D2E4794148AACBC26AA381CD7D30D");

    // task 4
    char *msg1 = "49206f776520796f75202432303030"; // M = I owe you $2000
    char *msg2 = "49206f776520796f75202433303030"; // M = I owe you $3000

    BIGNUM *sig1 = sign_hex(msg1, d, n, ctx);
    BIGNUM *sig2 = sign_hex(msg2, d, n, ctx);
    printBN("signature for \"I owe you $2000\":", sig1);
    printBN("signature for \"I owe you $3000\":", sig2);

    // Verify the signature
    //verify_hex(msg1, BN_bn2hex(sig1), e, n, ctx);
    //verify_hex(msg2, BN_bn2hex(sig2), e, n, ctx);

    // task 5
    BN_hex2bn(&n, "AE1CD4DC432798D933779FBD46C6E1247F0CF1233595113AA51B450F18116115");
    char *message   = "4c61756e63682061206d697373696c652e"; // "Launch a missile."
    char *signature = "643D6F34902D9C7EC90CB0B2BCA36C47FA37165C0005CAB026C0542CBDB6802F";
    bool verified = verify_hex(message, signature, e, n, ctx);
    if(verified) {
        printf("This is a valid message\n");
    }
    else {
        printf("This is not a valid message\n");
    }

    return 0;
}