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
#include "vlist.h"

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

void sortg(graph* g, dlow low[], size_t buff[]) {
	for (size_t v = 0; v < g->n; v++) {
		size_t v2;
		size_t i = 0;
		FORV(v2, g->v[v]) {
			buff[i++] = v2;
		}
		isort(buff, i, low, v);
		i = 0;
		size_t* a;
		FORVP(a, g->v[v]) {
			*a = buff[i++];
		}
	}
}

int planarity0(graph* g, dds* d, size_t v, dlow low[]) {
#define GETDS(X, Y) if ((X) < v) { 					\
		d->c[d->end++] = (constraint){1ull << (X), (X), 0, 69}; \
		Y = (ds){d->c+d->end-1, 1, (X)};			\
	}								\
	else {								\
		suppose(planarity0(g, d, (X), low)); 			\
		Y = d->v[X];						\
	}

	//should be impossible
	if (g->v[v].start == NULL) {
		d->v[v] = (ds){d->c+d->end, 0, 69};
	}
	else if (g->v[v].start->next == NULL) {
		GETDS(g->v[v].start->v, d->v[v]);
	}
	else {
		size_t v2 = g->v[v].start->v;
		GETDS(v2, d->v[v]);
		edge* e = g->v[v].start->next;
		suppose(force(&d->v[v], e->v < v ? e->v : low[e->v].a, &d->end));
		FORVC(v2, e) {
			ds d2;
			GETDS(v2, d2);
			suppose(force(&d2, d->v[v].l, &d->end));
			suppose(add(&d->v[v], &d2, &d->end));
		}
	}
	limit(&d->v[v], g->v[v].par, &d->end);
	return 1;
#undef GETDS
}

int planarity1(graph* g, dds* d, dlow low[], edge* elp[]) {	//yep goto state machine (a bit simpler then the previous)
	size_t v = 0;
	while (1) {
		size_t v2;
		if (g->v[v].start == NULL) {
			d->v[v] = (ds){d->c+d->end, 0, 69};
			goto back;
		}
		else if (g->v[v].start->next == NULL){
			v2 = g->v[v].start->v;
			if (v2 < v) {
				d->c[d->end++] = (constraint){1ull << v2, v2, 0, 69};
				d->v[v] = (ds){d->c+d->end-1, 1, v2};
				goto back;
			}
			else if (d->v[v2].start != NULL) {
				d->v[v] = d->v[v2];
				goto back;
			}
			else {
				v = v2;
				goto forw;
			}
		}
		else {
			ds d2;
			if (elp[v] == g->v[v].start) {
				d->v[v] = d->v[elp[v]->v];
				goto sloop;
			}
			if (elp[v]) goto loop;
			v2 = g->v[v].start->v;
			if (v2 < v) {
				d->c[d->end++] = (constraint){1ull << v2, v2, 0, 69};
				d->v[v] = (ds){d->c+d->end-1, 1, v2};
			}
			else {
				elp[v] = g->v[v].start;
				v = v2;
				goto forw;
			}
sloop:
			elp[v] = g->v[v].start->next;
			suppose(force(&d->v[v], elp[v]->v < v ? elp[v]->v : low[elp[v]->v].a, &d->end));
			FORVC(v2, elp[v]) {
				if (v2 < v) {
					d->c[d->end++] = (constraint){1ull << v2, v2, 0, 69};
					d2 = (ds){d->c+d->end-1, 1, v2};
				}
				else {
					v = v2;
					goto forw;
loop:
					v2 = elp[v]->v;
					d2 = d->v[v2];
				}
				suppose(force(&d2, d->v[v].l, &d->end));
				suppose(add(&d->v[v], &d2, &d->end));
			}
			goto back;
		}
back:
		limit(&d->v[v], g->v[v].par, &d->end);
		if (!v) break;
		v = g->v[v].par;
forw:
	}
	return 1;
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
	/*graph g = {
		malloc((6*n-12)*sizeof(edge)),
		0,
		malloc(n*sizeof(vertex)),
		n
	};*/
	graph g2 = {
		malloc((3*n-6)*sizeof(edge)),
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
		scanf("%s ", s);
		printf("%c%s\n", n+63, s);
		while (!feof(stdin)) {
			scanf("%s ", s);
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
			eprintf("%c%s\n", n+63, s);
			goto next;
		}
		if (e < 9) goto good;
		memset(g2.v, 0, sizeof(vertex)*n);
		g2.elen = 0;
		g2.n = n;
		if (!matdfs(g, n, e, low, ord, &g2)) {
			goto next;
		}
		sortg(&g2, low, ord);
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
