#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <signal.h>
#include "buckets.h"
#include "common.h"

#include "printing.h"
#include "matrix.h"
//#include "vlist.h"

void isort(size_t arr[], size_t len, dlow low[], size_t v0) {	//insertion sort for vertices (should switch to something faster (heap sort) when i can)
	for (size_t i = 1; i < len; i++) {
		size_t v = arr[i];
		int j = i-1;
		for (;j >= 0; j--) {
			size_t a = v < v0 ? v : low[v].a;
			size_t b = arr[j] < v0 ? arr[j] : low[arr[j]].a;
			if (a > b) break;
			if (a == b) {
				if (v < v0 || arr[j] < v0) {
					if (v > arr[j]) break;
				}
				else {
					if (low[v].b < low[arr[j]].b) break;
				}
			}
			arr[j+1] = arr[j];
		}
		arr[j+1] = v;
	}
}

void sortg(graph* g, dlow low[]) {
	for (size_t v = 0; v < g->n; v++) {
		isort(g->v[v].start, g->v[v].len, low, v);
	}
}

int planarity0(graph* g, dds* d, size_t v, dlow low[]) {	//better and simpler
#define GETDS(X, Y) 							\
	if ((X) < v) { 							\
		d->c[d->end++] = (constraint){1ull << (X), (X), 0, 69}; \
		Y = (ds){d->c+d->end-1, 1, (X)};			\
	}								\
	else {								\
		suppose(planarity0(g, d, (X), low)); 			\
		Y = d->v[X];						\
	}

	if (g->v[v].len == 0) {		//should be impossible
		d->v[v] = (ds){d->c+d->end, 0, 69};
	}
	else if (g->v[v].len == 1) {
		GETDS(g->v[v].start[0], d->v[v]);
	}
	else {
		size_t v2 = g->v[v].start[0];
		GETDS(v2, d->v[v]);
		v2 = g->v[v].start[1];
		suppose(force(&d->v[v], v2 < v ? v2 : low[v2].a, &d->end));
		for (size_t i = 1; i < g->v[v].len; i++) {
			ds d2;
			v2 = g->v[v].start[i];
			GETDS(v2, d2);
			suppose(force(&d2, d->v[v].l, &d->end));
			suppose(add(&d->v[v], &d2, &d->end));
		}
	}
	limit(&d->v[v], g->v[v].par, &d->end);
	return 1;
#undef GETDS
}

int getn(FILE* fin) {
	char n = getc(fin);
	assert(n-63);
	return n - 63;
}

int main() {
	int n = getn(stdin);
	size_t len = (n*(n-1)/2+5)/6;
	char* s = malloc(len+2+7);
	memset(s+len, 0x3f, 8);
	gmat g[64] = {0};
	graph g2 = {
		malloc((6*n)*sizeof(edge)),
		0,
		malloc(n*sizeof(vertex)),
		n
	};
	dlow* low = malloc(n*sizeof(dlow));
	size_t* ord = malloc(n*sizeof(size_t));
	edge** elp = malloc(n*sizeof(edge*));
	dds dss = {
		.c=malloc((3*n-6)*sizeof(constraint)),
		.v=malloc(n*sizeof(ds)),
		.end=0
	};
	if (n < 5) {
		assert(scanf("%s ", s));
		printf("%c%s\n", n+63, s);
		while (!feof(stdin)) {
			assert(scanf("%s ", s));
			printf("%s\n", s);
		}
		return 0;
	}
	while (!feof(stdin)) {
		//memset(g.v, 0, sizeof(vertex)*n);
		memset(g, 0, sizeof(gmat)*64);
		//g.elen = 0;
		int e = getmat(stdin, g, s, n);
		if (e > 3*n-6) {
			goto next;
		}
		if (e < 9) goto good;
		memset(g2.v, 0, sizeof(vertex)*n);
		g2.elen = 0;
		g2.n = n;
		if (!matdfs(g, n, e, low, ord, &g2)) {
			goto next;
		}
		if (g2.elen < g2.n+3) {
			goto good;
		}
		sortg(&g2, low);
		memset(dss.c, 0, sizeof(constraint)*(3*n-6));
		memset(dss.v, 0, sizeof(ds)*n);
		dss.end = 0;
		if (planarity0(&g2, &dss, 0, low)) {
good:
			fputc(n+63, stdout);
			fputs(s, stdout);
		}
next:
		if (feof(stdin)) break;
		getn(stdin);
	}
}
