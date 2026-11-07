#include <stdio.h>

long long File__get_size(char *filename) {
	FILE *file = fopen(filename, "rb");

	if (file == NULL) {
		perror("fopen");
		return 1;
	}

	fseek(file, 0, SEEK_END);
	long long length = ftell(file);

	fclose(file);

	return length;
}

void File__read_file(char *filename, char *result) {
	FILE *file = fopen(filename, "r");

	if(file) {
		fread(result, 1, File__get_size(filename), file);
		fclose(file);
	} else {
		perror("fopen");
	}
}