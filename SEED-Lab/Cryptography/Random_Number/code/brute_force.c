#include <stdio.h>
#include <string.h>
#include <openssl/evp.h>

#define BLOCK_SIZE 16
#define KEY_SIZE 16
#define IV_SIZE 16

void print_bytes(unsigned char *arr, int len) {
    for(int i = 0; i < len; i++) {
        printf("%02x ", arr[i]);
    }
    printf("\n");
}
int cmp_ciphertext(unsigned char *C1, unsigned char *C2, int len1, int len2) {
    if(len1 != len2) {
        return 0;
    }
    for(int i = 0; i < len1; i++) {
        if(C1[i] != C2[i]) {
            return 0;
        }
    }
    return 1;
}

int brute_force(unsigned char *plaintext, unsigned char *ciphertext, unsigned char *iv) {
    int do_encrypt = 1;
    int  pt_len = BLOCK_SIZE, ct_len = BLOCK_SIZE;
    

    unsigned char key[KEY_SIZE];
    for(long long ts = 1524013729; ts <= 1524020929; ts++) {
        // Generate key using the timestamp
        srand(ts);
        for(int i = 0; i < KEY_SIZE; i++) {
            key[i] = rand() % 256;
            printf("%.2x", (unsigned char) key[i]);
        }
        printf("\n");
        // print_bytes(key, KEY_SIZE);
        // print_key_ascii(key);
        
        int outlen;
        unsigned char outbuf[1024 + EVP_MAX_BLOCK_LENGTH];
        EVP_CIPHER_CTX *ctx;

        // Don't set key or IV right away; we want to check lengths
        ctx = EVP_CIPHER_CTX_new();
        EVP_CipherInit_ex(ctx, EVP_aes_128_cbc(), NULL, NULL, NULL, do_encrypt);
        OPENSSL_assert(EVP_CIPHER_CTX_key_length(ctx) == KEY_SIZE);
        OPENSSL_assert(EVP_CIPHER_CTX_iv_length(ctx) == IV_SIZE);
        // Now we can set key and IV
        EVP_CipherInit_ex(ctx, NULL, NULL, key, iv, do_encrypt);
        
        // We only need to encrypt the first block
        if(!EVP_CipherUpdate(ctx, outbuf, &outlen, plaintext,  pt_len)) {
            EVP_CIPHER_CTX_free(ctx);
            return 0;
        }
        print_bytes(outbuf, outlen);
        
        // Get the target ciphertext
        if(cmp_ciphertext(ciphertext, outbuf, ct_len, outlen) == 1) {
            printf("Find the key:\n");
            print_bytes(key, KEY_SIZE);
            printf("Timestamp:%lld\n", ts);
            return 1;
        }
        
        EVP_CIPHER_CTX_free(ctx);
    }
    
    printf("No matching key in the keys file\n");
    return 0;
}


int main() {
    unsigned char P1[] = {
        0x25, 0x50, 0x44, 0x46,
        0x2d, 0x31, 0x2e, 0x35,
        0x0a, 0x25, 0xd0, 0xd4,
        0xc5, 0xd8, 0x0a, 0x34
    };
    unsigned char C1[] = {
    	0xd0, 0x6b, 0xf9, 0xd0,
    	0xda, 0xb8, 0xe8, 0xef,
    	0x88, 0x06, 0x60, 0xd2,
    	0xaf, 0x65, 0xaa, 0x82
    };
    unsigned char IV[] = {
    	0x09, 0x08, 0x07, 0x06,
    	0x05, 0x04, 0x03, 0x02,
    	0x01, 0x00, 0xa2, 0xb2,
    	0xc2, 0xd2, 0xe2, 0xf2
    };

    brute_force(P1, C1, IV);

    return 0;
}
