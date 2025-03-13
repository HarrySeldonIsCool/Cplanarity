#include <stdio.h>

void printeset(FILE* fout, eset e) {
	fprintf(fout, "[");
	for (size_t i = 0; i < 64; i++) {
		if (e.e >> i & 1) {
			fprintf(fout, "%li, ", i);
		}
	}
	if (e.e) fprintf(fout, "\b\b");
	fprintf(fout, "]: %li", e.l);
}

void printcons(FILE* fout, constraint* c) {
	fprintf(fout, "(");
	printeset(fout, c->r);
	fprintf(fout, ", ");
	printeset(fout, c->l);
	fprintf(fout, ")");
}

void printds(FILE* fout, ds* d) {
	fprintf(fout, "DS { c: [");
	for (size_t i = 0; i < d->len; i++) {
		printcons(fout, &d->start[i]);
		fprintf(fout, ", ");
	}
	if (d->len) fprintf(fout, "\b\b");
	fprintf(fout, "], l: %li}\n", d->l);
}

void printg(FILE* fout, graph* g) {
	for (size_t v = 0; v < g->n; v++) {
		size_t v2;
		fprintf(fout, "%li: ", v);
		for (size_t i = 0; i < g->v[v].len; i++) {
			fprintf(fout, "%li ", g->v[v].start[i]);
		}
		fprintf(fout, "\n");
	}
}

void printord(FILE* fout, size_t* a, size_t len) {
	fprintf(fout, "[\n");
	for (size_t i = 0; i < len; i++) {
		fprintf(fout, "\t%li => %li\n", i, a[i]);
	}
	fprintf(fout, "]\n");
}

void printlow(FILE* fout, dlow* a, size_t len) {
	fprintf(fout, "[\n");
	for (size_t i = 0; i < len; i++) {
		fprintf(fout, "\t%li => (%li, %li)\n", i, a[i].a, a[i].b);
	}
	fprintf(fout, "]\n");
}
