#include <stdio.h>
#include <string.h>
#include <openssl/evp.h>

#define KEY_SIZE 16
#define IV_SIZE 16

void print_bytes(unsigned char *arr, int len) {
    for(int i = 0; i < len; i++) {
        printf("%02x ", arr[i]);
    }
    printf("\n");
}
void print_key_ascii(unsigned char *key) {
    for(int i = 0; i < KEY_SIZE; i++) {
        printf("%c", key[i]);
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

int do_encrypt(FILE *in, FILE *out, unsigned char *key, unsigned char *iv) {
    int do_encrypt = 1;
    // Allow enough space in output buffer for additional block
    unsigned char inbuf[1024], outbuf[1024 + EVP_MAX_BLOCK_LENGTH];
    int inlen, outlen;
    EVP_CIPHER_CTX *ctx;

    // Don't set key or IV right away; we want to check lengths
    ctx = EVP_CIPHER_CTX_new();
    EVP_CipherInit_ex(ctx, EVP_aes_128_cbc(), NULL, NULL, NULL, do_encrypt);
    OPENSSL_assert(EVP_CIPHER_CTX_key_length(ctx) == 16);
    OPENSSL_assert(EVP_CIPHER_CTX_iv_length(ctx) == 16);
    // Now we can set key and IV
    EVP_CipherInit_ex(ctx, NULL, NULL, key, iv, do_encrypt);
    for(;;) {
        inlen = fread(inbuf, 1, 1024, in);
        if(inlen <= 0) break;
        if(!EVP_CipherUpdate(ctx, outbuf, &outlen, inbuf, inlen)) {
            EVP_CIPHER_CTX_free(ctx);
            return 0;
        }
        print_bytes(outbuf, outlen);
        fwrite(outbuf, 1, outlen, out);
    }
    if (!EVP_CipherFinal_ex(ctx, outbuf, &outlen)) {
        EVP_CIPHER_CTX_free(ctx);
        return 0;
    }
    print_bytes(outbuf, outlen);
    fwrite(outbuf, 1, outlen, out);

    EVP_CIPHER_CTX_free(ctx);
    return 1;
}

int brute_force(FILE *in, FILE *target, FILE *keys, unsigned char *iv) {
    int do_encrypt = 1;
    // Allow enough space in output buffer for additional block
    int inlen, ct_len;
    unsigned char inbuf[1024], ciphertext[1024];
    inlen = fread(inbuf, 1, 1024, in);
    ct_len = fread(ciphertext, 1, 1024, target);
    printf("ctlen: %d\n", ct_len);
    // print_bytes(ciphertext, ct_len);
    
    // In words.txt, there are words longer than KEY_SIZE, 
    // to prevent overflow, use a larger buffer
    unsigned char key[KEY_SIZE], key_buf[256];
    while(fgets(key_buf, 256, keys) != NULL) {
        int key_len = strlen(key_buf);
        // Skip the word if it is longer than KEY_SIZE
        if(key_len > KEY_SIZE) {
            continue;
        }
        // Pad the key to 16 bytes with 0x23 ('#')
        for(int i = 0; i < KEY_SIZE; i++) {
            if(i < key_len - 1) {
                key[i] = key_buf[i];
            }
            else {
                key[i] = 0x23;
            }
        }
        // print_bytes(key, KEY_SIZE);
        // print_key_ascii(key);
        
        int outlen1, outlen2;
        unsigned char outbuf[1024 + EVP_MAX_BLOCK_LENGTH];
        EVP_CIPHER_CTX *ctx;

        // Don't set key or IV right away; we want to check lengths
        ctx = EVP_CIPHER_CTX_new();
        EVP_CipherInit_ex(ctx, EVP_aes_128_cbc(), NULL, NULL, NULL, do_encrypt);
        OPENSSL_assert(EVP_CIPHER_CTX_key_length(ctx) == KEY_SIZE);
        OPENSSL_assert(EVP_CIPHER_CTX_iv_length(ctx) == IV_SIZE);
        // Now we can set key and IV
        EVP_CipherInit_ex(ctx, NULL, NULL, key, iv, do_encrypt);
        
        if(!EVP_CipherUpdate(ctx, outbuf, &outlen1, inbuf, inlen)) {
            EVP_CIPHER_CTX_free(ctx);
            return 0;
        }
        
        if(!EVP_CipherFinal_ex(ctx, outbuf + outlen1, &outlen2)) {
            EVP_CIPHER_CTX_free(ctx);
            return 0;
        }

        // print_bytes(outbuf, outlen1 + outlen2);

        // Get the target ciphertext
        if(cmp_ciphertext(ciphertext, outbuf, ct_len, outlen1 + outlen2) == 1) {
            printf("Find the key:\n");
            print_key_ascii(key);
            return 1;
        }
        
        EVP_CIPHER_CTX_free(ctx);
    }
    
    printf("No matching key in the keys file\n");
    return 0;
}


int main() {
    // For this task, we can directly define the data here
    // But putting them in a file will be more convenient
    unsigned char P1[] = "This is a top secret.";
    unsigned char C1[] = {
    	0x76, 0x4a, 0xa2, 0x6b, 0x55, 0xa4, 0xda, 0x65, 
    	0x4d, 0xf6, 0xb1, 0x9e, 0x4b, 0xce, 0x00, 0xf4, 
    	0xed, 0x05, 0xe0, 0x93, 0x46, 0xfb, 0x0e, 0x76, 
    	0x25, 0x83, 0xcb, 0x7d, 0xa2, 0xac, 0x93, 0xa2
    };
    unsigned char iv[] = {
    	0xaa, 0xbb, 0xcc, 0xdd, 
    	0xee, 0xff, 0x00, 0x99, 
    	0x88, 0x77, 0x66, 0x55, 
    	0x44, 0x33, 0x22, 0x11
    };

    unsigned char test_key[KEY_SIZE] = {
        0x01, 0x02, 0x03, 0x04, 
        0x05, 0x06, 0x07, 0x08, 
        0x23, 0x23, 0x23, 0x23, 
        0x23, 0x23, 0x23, 0x23
    };

    // FILE *pt, *ct;
    // pt = fopen("plain.txt", "r");
    // ct = fopen("cipher.bin", "w+");
    // do_encrypt(p, c, test_key, iv);
    
    FILE *plaintext, *ciphertext, *keys;
    plaintext  = fopen("plain.txt", "r");
    ciphertext = fopen("ciphertext.bin", "r");
    keys       = fopen("words.txt", "r");
    brute_force(plaintext, ciphertext, keys, iv);

    fclose(plaintext);
    fclose(ciphertext);
    fclose(keys);

    return 0;
}
