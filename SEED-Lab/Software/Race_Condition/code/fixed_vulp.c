#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main() {
	char* fn = "/tmp/XYZ";
	char buffer[60];
	FILE* fp;

	/* get user input */
	scanf("%50s", buffer);

	setuid(getuid()); // temporarily revoke the root privilege
	system("/bin/id");

	fp = fopen(fn, "a+");
	if(!fp) {
		perror("Open failed");
		exit(1);
	}
	fwrite("\n", sizeof(char), 1, fp);
	fwrite(buffer, sizeof(char), strlen(buffer), fp);
	fclose(fp);

	setuid(0); // try to regain root privilege, but will not success
	system("/bin/id");

	return 0;
}
