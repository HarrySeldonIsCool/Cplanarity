#include <stdio.h>

int main(int argc, char** argv) {
	FILE* fin, *fout;
	if (argc > 1) {
		fin = fopen(argv[1], "r");
	} else {
		fin = stdin;
	}
	if (argc > 2) {
		fout = fopen(argv[2], "w");
	} else {
		fout = stdout;
	}
	char buff[100];
	fscanf(fin, "%s ", buff);
	int n = buff[0]-63;
	fprintf(fout, "%c", buff[0]);
	uint64_t g[64];
	size_t x = 0;
	while (1) {
		for (size_t i = 0; i < n; i++) {
			for (size_t j = 0; j < i; j++) {
				g[i] |= (buff[x/6]-63 >> 5-x%6 & 0x1) << j;
				g[j] |= (buff[x/6]-63 >> 5-x%6 & 0x1) << i;
				x++;
			}
		}
		for (size_t i = 0; i < n; i++) {
			for (size_t j = 0; j < n; j += 6) {
				putc(fout, (g[i] >> j & 0x3f) + 64);
			}
		}
		if (feof(fin)) break;
		fscanf(fin, "%s ", buff);
	}
	fclose(fin);
	fclose(fout);
	return 0;
}
