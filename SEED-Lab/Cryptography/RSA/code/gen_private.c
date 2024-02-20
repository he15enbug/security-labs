#include <stdio.h>
#include <openssl/bn.h>

void printBN(char *msg, BIGNUM * a) {
    /* Use BN_bn2hex(a) for hex string
     * Use BN_bn2dec(a) for decimal string */
    char * number_str = BN_bn2hex(a);
    printf("%s %s\n", msg, number_str);
    OPENSSL_free(number_str);
}

int main () {
    BN_CTX *ctx = BN_CTX_new();

    BIGNUM *p = BN_new();
    BIGNUM *q = BN_new();
    BIGNUM *e = BN_new();
    BN_hex2bn(&p, "F7E75FDC469067FFDC4E847C51F452DF");
    BN_hex2bn(&q, "E85CED54AF57E53E092113E62F436F4F");
    BN_hex2bn(&e, "0D88C3");

    // BIGNUM *n = BN_new();
    // BN_mul(n, p, q, ctx);

    BIGNUM *one = BN_new();
    BIGNUM *p_sub_1 = BN_new();
    BIGNUM *q_sub_1 = BN_new();
    BIGNUM *phi_n = BN_new();
    BN_hex2bn(&one, "1");
    BN_sub(p_sub_1, p, one);
    BN_sub(q_sub_1, q, one);
    BN_mul(phi_n, p_sub_1, q_sub_1, ctx);

    BIGNUM *private_key = BN_new();
    BN_mod_inverse(private_key, e, phi_n, ctx);

    printBN("private key = ", private_key);

    return 0;
}