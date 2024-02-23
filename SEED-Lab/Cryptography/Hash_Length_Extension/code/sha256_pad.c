#include <stdint.h>
#include <stdio.h>
#include <string.h>

// Function to perform padding for SHA-256
void sha256_pad(char *message, size_t *length) {
    // Calculate the number of padding bytes needed
    size_t original_length = *length;
    size_t padding_length = 64 - ((*length + 8) % 64);
    if(padding_length < 1) {
        padding_length += 64;
    }
    // Append a byte 0x80
    message[(*length)++] = 0x80;
    // Append 0x00
    for(int i = 1; i < padding_length; i++) {
        message[(*length)++] = 0x00;
    }
    // Append the original length in bits as a 64-bit big-endian integer
    for(int i = 0; i < 8; ++i) {
        message[(*length)++] = (original_length * 8) >> (56 - i * 8);
    }
}

int main() {
    unsigned char message[6400] = "983abe:myname=he15enbug&uid=1002&lstcmd=1";
    size_t length = strlen(message);

    sha256_pad(message, &length);

    printf("Padded message:\n");
    for (int i = 0; i < length; ++i) {
        printf("\\x%02x", message[i]);
    }
    printf("\n");
    printf("Padded message (URL encoding):\n");
    for (int i = 0; i < length; ++i) {
        printf("%%%02x", message[i]);
    }
    printf("\n");
    return 0;
}