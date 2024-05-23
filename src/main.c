#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <signal.h>
#include "buckets.h"

typedef struct edge_s {
	struct edge_s* next;
	size_t v;
} edge;

typedef struct {
	edge* start;
	size_t par;
} vertex;

typedef struct {
	edge* e;
	size_t elen;
	vertex* v;
	size_t n;
} graph;

#define CONCI(A, B) A ## B
#define CONCAT(A, B) CONCI(A, B)
#define FORV(A, V) for (edge* CONCAT(next, __LINE__) = V.start; CONCAT(next, __LINE__) && ((A = CONCAT(next, __LINE__)->v) || 1); CONCAT(next, __LINE__) = CONCAT(next, __LINE__)->next) //predefine A
#define FORVC(A, E) for (;E && ((A = E->v) || 1); E = E->next) //predeclare A and E
#define FORVP(A, V) for (edge* CONCAT(next, __LINE__) = V.start; CONCAT(next, __LINE__) && ((A = &CONCAT(next, __LINE__)->v) || 1); CONCAT(next, __LINE__) = CONCAT(next, __LINE__)->next)
#define ok )) return 0
#define is if (!(
#define kk ?1:exit(__LINE__)
#define eprintf(...) fprintf(stderr, __VA_ARGS__)

void pushg(graph* g, size_t v, size_t v2) {
	g->e[g->elen] = (edge){g->v[v].start, v2};
	g->v[v].start = &g->e[g->elen++];
	return;
}

typedef struct {
	size_t a;
	size_t b;
} dlow;

static volatile char sdump[100];

void dump_on_sig(int dum) {
	eprintf("%s", sdump);
	raise(SIGABRT);
}

//abusing the parent field in g - "stackless"
//g2 is directed tree with set parents
#define MIN(A, B) ((A) < (B) ? (A) : (B))
#define MAX(A, B) ((A) > (B) ? (A) : (B))
void dfs(graph* g, dlow low[], size_t ord[], graph* g2, edge** elp) {
	uint64_t exp = 1;
	for (size_t i = 0; i < g->n; i++) {
		low[i] = (dlow){i, i};
		elp[i] = g->v[i].start;
	}
	size_t v = 0;
	size_t top = 0;
	while (1) {
		size_t v2;
		size_t t = ord[v];
		FORVC(v2, elp[v]) {
			size_t tx = ord[v2];
			if (1 << v2 & exp) {
				if (tx < g2->v[t].par) {
					pushg(g2, t, tx);
					if (tx < low[t].b && tx != low[t].a) {
						low[t].b = MAX(tx, low[t].a);
						low[t].a = MIN(tx, low[t].a);
					}
				}
			}
			else {
				exp |= 1 << v2;
				g->v[v2].par = v;
				ord[v2] = ++top;
				g2->v[top].par = t;
				pushg(g2, t, top);
				elp[v] = elp[v]->next;
				v = v2;
				goto next;
			}
		}
		if (!v) return;
		v = g->v[v].par;	//backtrack
		size_t tp = ord[v];
		if (low[t].a < low[tp].b && low[t].a != low[tp].a) {
			low[tp].b = MAX(low[t].a, low[tp].a);
			low[tp].a = MIN(low[t].a, low[tp].a);
		}
		low[tp].b = MIN(low[tp].b, low[t].b);
next:
	}
}

void isort(size_t arr[], size_t len, dlow low[]) {	//insertion sort for vertices (should switch to something faster (heap sort) when i can)
	for (size_t i = 1; i < len; i++) {
		size_t v = arr[i];
		int j = i-1;
		for (;j >= 0; j--) {
			size_t a = v < i ? v : low[v].a;
			size_t b = arr[j] < i ? arr[j] : low[arr[j]].a;
			if (a > b) break;
			if (a == b) {
				if (v < i || arr[j] < i) {
					if (v > arr[j]) break;
				}
				else {
					if (low[v].b < low[arr[j]].b) break;
					if (low[v].b == low[arr[j]].b && v > arr[j]) break;
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
		isort(buff, i, low);
		i = 0;
		size_t* a;
		FORVP(a, g->v[v]) {
			*a = buff[i++];
		}
	}
}

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

int planarity0(graph* g, dds* d, size_t v, dlow low[]) {
#define GETDS(X, Y) if ((X) < v) { \
	d->c[d->end++] = (constraint){1ll << (X), (X), 0, 69}; \
	Y = (ds){d->c+d->end-1, 1, (X)};\
	}\
	else {\
		is planarity0(g, d, (X), low) ok; \
		Y = d->v[X];\
	}
	if (g->v[v].start == NULL) {
		d->v[v] = (ds){d->c+d->end, 0, 69};
	}
	else if (g->v[v].start->next == NULL) {
		GETDS(g->v[v].start->v, d->v[v]);
		limit(&d->v[v], g->v[v].par, &d->end);
	}
	else {
		size_t v2 = g->v[v].start->v;
		GETDS(v2, d->v[v]);
		is force(&d->v[v], v2 < v ? v2 : low[v2].b, &d->end) ok;
		edge* e = g->v[v].start->next;
		FORVC(v2, e) {
			ds d2;
			GETDS(v2, d2);
			is force(&d2, d->v[v].l, &d->end) ok;
			is add(&d->v[v], &d2, &d->end) ok;
		}
	}
	limit(&d->v[v], g->v[v].par, &d->end);
	return 1;
#undef GETDS
}

int planarity2(graph* g, dds* d, edge** elp, ds* elp2, dlow low[]) { //fucking goto state machine, kill me already
	size_t v = 0;
	while(1) {
		size_t v2;
		if (g->v[v].start == NULL) {
			elp2[g->v[v].par] = d->v[v] = (ds){d->c+d->end, 0, 69};
		}
		else if (g->v[v].start->next == NULL) {
			if (elp[v] == NULL) {
				elp[v] = g->v[v].start;
				v2 = elp[v]->v;
				if (v2 < v) {
					d->c[d->end++] = (constraint){1ll << v2, v2, 0, 69};
					elp2[v] = (ds){d->c+d->end-1, 1, v2};
				}
				else {	//recurse
					v = v2;
					goto next;
				}
			}
			limit(&elp2[v], g->v[v].par, &d->end);
			elp2[g->v[v].par] = elp2[v];
		}
		else {
			if (elp[v] == NULL) goto first;
			else if (elp[v] == g->v[v].start) goto forces;
			else goto rest;
first:			elp[v] = g->v[v].start;
			if (elp[v]->v < v) {
				d->c[d->end++] = (constraint){1ll << elp[v]->v, elp[v]->v, 0, 69};
				elp2[v] = (ds){d->c+d->end-1, 1, elp[v]->v};
			}
			else {	//recurse
				v = elp[v]->v;
				goto next;
			}
forces:			d->v[v] = elp2[v];
			elp[v] = elp[v]->next;
			is force(&d->v[v], elp[v]->v < v ? elp[v]->v : low[elp[v]->v].b, &d->end) ok;
			FORVC(v2, elp[v]) {
				if (v2 < v) {
					d->c[d->end++] = (constraint){1ll << v2, v2, 0, 69};
					elp2[v] = (ds){d->c+d->end-1, 1, v2};
				}
				else {
					v = v2;
					goto next;
				}
rest:				is force(&elp2[v], d->v[v].l, &d->end) ok;
				is add(&d->v[v], &elp2[v], &d->end) ok;
			}
			limit(&d->v[v], g->v[v].par, &d->end);
			elp2[g->v[v].par] = d->v[v];
		}
		if (!v) break;
		//backtrack
		v = g->v[v].par;
		next:
	}
	return 1;
}

int getg(FILE* fin, graph* g, char* s, int n) {
	assert(fscanf(fin, "%s ", s));
	sprintf(sdump, "%c%s\n", n+63, s);
	size_t x = 0;
	for (size_t i = 0; i < n; i++) {
		for (size_t j = 0; j < i; j++) {
			if (s[x/6]-63 << x%6 & 0x20) {
				if (g->elen >= 6*n-12) return 0;
				pushg(g, i, j);
				pushg(g, j, i);
			}
			x++;
		}
	}
	return 1;
}

int getn(FILE* fin) {
	char n = getc(fin);
	assert(n-63);
	return n - 63;
}

void printg(FILE* fout, graph* g) {
	for (size_t v = 0; v < g->n; v++) {
		size_t v2;
		fprintf(fout, "%li: ", v);
		FORV(v2, g->v[v]) {
			fprintf(fout, "%li ", v2);
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

int main() {
	signal(SIGSEGV, dump_on_sig);
	int n = getn(stdin);
	char* s = malloc(n*(n-1)/12+3);
	graph g = {
		malloc((6*n-12)*sizeof(edge)),
		0,
		malloc(n*sizeof(vertex)),
		n
	};
	graph g2 = {
		malloc((6*n-12)*sizeof(edge)),
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
	ds* elp2 = calloc(n, sizeof(ds));
	while (!feof(stdin)) {
		memset(s, 0, n*(n-1)/12+3);
		memset(g.e, 0, sizeof(edge)*(6*n-12));
		memset(g.v, 0, sizeof(vertex)*n);
		g.elen = 0;
		if (!getg(stdin, &g, s, n)) goto next;
		memset(g2.e, 0, sizeof(edge)*(6*n-12));
		memset(g2.v, 0, sizeof(vertex)*n);
		g2.elen = 0;
		memset(low, 0, sizeof(dlow)*n);
		memset(ord, 0, sizeof(size_t)*n);
		memset(elp, 0, sizeof(edge*)*n);
		dfs(&g, low, ord, &g2, elp);
		sortg(&g2, low, ord);
		memset(dss.c, 0, sizeof(constraint)*(3*n-6));
		memset(dss.v, 0, sizeof(ds)*n);
		dss.end = 0;
		if (planarity0(&g2, &dss, 0, low)) {
			printf("%c%s\n", n+63, s);
		}
next:
		if (feof(stdin)) break;
		assert(n == getn(stdin));
	}
}