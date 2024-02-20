#include <stdio.h>
#include <openssl/bn.h>
#include <stdbool.h>

void printBN(char *msg, BIGNUM * a) {
    char * number_str = BN_bn2hex(a);
    printf("%s\n%s\n", msg, number_str);
    OPENSSL_free(number_str);
}

bool verify(BIGNUM *message, BIGNUM *signature, BIGNUM *e, BIGNUM *n, BN_CTX *ctx) {
    BIGNUM *msg = BN_new();
    BN_mod_exp(msg, signature, e, n, ctx);
    printBN("original message being signed:", msg);
    return (BN_cmp(message, msg) == 0);
}

void verify_hex(char *message_hex, char *signature_hex, BIGNUM *e, BIGNUM *n, BN_CTX *ctx) {
    BIGNUM *message = BN_new();
    BN_hex2bn(&message, message_hex);
    BIGNUM *signature = BN_new();
    BN_hex2bn(&signature, signature_hex);

    BIGNUM *msg = BN_new();
    BN_mod_exp(msg, signature, e, n, ctx);
    printBN("**********Message Being Signed**********", msg);
    printBN("**********Hash of the Server's Certificate**********", message);
}

int main () {
    BN_CTX *ctx = BN_CTX_new(); 

    BIGNUM *e = BN_new();
    BIGNUM *n = BN_new();
    BN_hex2bn(&n, "BB021528CCF6A094D30F12EC8D5592C3F882F199A67A4288A75D26AAB52BB9C54CB1AF8E6BF975C8A3D70F4794145535578C9EA8A23919F5823C42A94E6EF53BC32EDB8DC0B05CF35938E7EDCF69F05A0B1BBEC094242587FA3771B313E71CACE19BEFDBE43B45524596A9C153CE34C852EEB5AEED8FDE6070E2A554ABB66D0E97A540346B2BD3BC66EB66347CFA6B8B8F572999F830175DBA726FFB81C5ADD286583D17C7E709BBF12BF786DCC1DA715DD446E3CCAD25C188BC60677566B3F118F7A25CE653FF3A88B647A5FF1318EA9809773F9D53F9CF01E5F5A6701714AF63A4FF99B3939DDC53A706FE48851DA169AE2575BB13CC5203F5ED51A18BDB15");
    BN_hex2bn(&e, "10001");

    char *cert_hash = "06f31cb82f47856dd892091ce5e34bb7680e07ccf8f9ca5138ee782b2cddc784";
    char *signature = "3ae552eb9e8a66f361ef54e927723b2a93e2b397ef46970bf630cc6809200fb6a5a80a1ca081d1a0bb67300b75b4cca8b2b5c4b5b00ae484d6bf894b2d8e2c118a824ab907f165cb8b8f14191c5912ae899fb923ff2fbdb43068259e3fa39e04e2bfa5692f08d6a13026436b13f2960480352a8c97e964a31b490851ee64f6a7ed0e9ed23533236dd1c02cf5e688cf4fc03f737518be8738d8fb3f4882e8f861d16f2a037d3ed1b0c2e1bc834c6e1736285103dfda9929d8226827b3905fe8174fbb1355ecfcca5b277c8f8cbde4e9f46fc76574a122ab3a394e749a5db8c2ceb9dbdccdca0413be9854f66e61ff5be232da494d48ad500220b25617b66e5782";
    verify_hex(cert_hash, signature, e, n, ctx);

    return 0;
}