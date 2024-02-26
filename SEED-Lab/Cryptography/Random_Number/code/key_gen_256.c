#include <stdio.h>
#include <stdlib.h>

#define LEN 32

void print_bytes(unsigned char *arr, int len) {
    for(int i = 0; i < len; i++) {
        printf("%02x ", arr[i]);
    }
    printf("\n");
}

int main() {
    unsigned char *key = (unsigned char *) malloc(sizeof(unsigned char) * LEN);
    
    FILE* random = fopen("/dev/urandom", "r");
    fread(key, sizeof(unsigned char) * LEN, 1, random);
    
    print_bytes(key, LEN);
    
    fclose(random);
    return 0;
}