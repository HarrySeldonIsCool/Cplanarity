#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
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
#define FORVC(A, E) for (;E && ((A = E->v) || 1); E = E->next) //prefine A and E
#define FORVP(A, V) for (edge* CONCAT(next, __LINE__) = V.start; CONCAT(next, __LINE__) && ((A = &CONCAT(next, __LINE__)->v) || 1); CONCAT(next, __LINE__) = CONCAT(next, __LINE__)->next)
#define ok )) {return 0;}
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

#include "printing.h"

//abusing the parent field in g - "stackless"
//g2 is directed tree with set parents
#define MIN(A, B) ((A) < (B) ? (A) : (B))
#define MAX(A, B) ((A) > (B) ? (A) : (B))
int dfs(graph* g, dlow low[], size_t ord[], graph* g2, edge** elp) {
	uint64_t exp = 0;
	for (size_t i = 0; i < g->n; i++) {
		low[i] = (dlow){i, i};
		elp[i] = g->v[i].start;
	}
	int top = -1;
	for (size_t i = 0; i < g->n; i++) {
		if (~exp & 1ull << i) {
			size_t edges = 0;
			int otop = top;
			size_t v = i;
			ord[v] = ++top;
			g2->v[top].par = 0;
			exp |= 1ull << i;
			while (1) {
				size_t v2;
				size_t t = ord[v];
				FORVC(v2, elp[v]) {
					size_t tx = ord[v2];
					if (1ull << v2 & exp) {
						if (tx < g2->v[t].par) {
							pushg(g2, t, tx);
							edges++;
							if (tx < low[t].b && tx != low[t].a) {
								low[t].b = MAX(tx, low[t].a);
								low[t].a = MIN(tx, low[t].a);
							}
						}
					}
					else {
						exp |= 1ull << v2;
						g->v[v2].par = v;
						ord[v2] = ++top;
						g2->v[top].par = t;
						pushg(g2, t, top);
						edges++;
						elp[v] = elp[v]->next;
						v = v2;
						goto next;
					}
				}
				if (v == i) break;
				v = g->v[v].par;	//backtrack
				size_t tp = ord[v];
				if (low[t].a < low[tp].b && low[t].a != low[tp].a) {
					low[tp].b = MAX(low[t].a, low[tp].a);
					low[tp].a = MIN(low[t].a, low[tp].a);
				}
				low[tp].b = MIN(low[tp].b, low[t].b);
next:
			}
			int n = top-otop;
			if (edges > 3*(n-2) && n > 2) return 0;
			if (otop+1 && n+3 <= edges && n >= 5 && edges >= 9) pushg(g2, 0, otop+1);
		}
	}
	return 1;
}

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
		isort(buff, i, low, v);
		i = 0;
		size_t* a;
		FORVP(a, g->v[v]) {
			*a = buff[i++];
		}
	}
}

int planarity0(graph* g, dds* d, size_t v, dlow low[]) {
#define GETDS(X, Y) if ((X) < v) { \
	d->c[d->end++] = (constraint){1ull << (X), (X), 0, 69}; \
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
	}
	else {
		size_t v2 = g->v[v].start->v;
		GETDS(v2, d->v[v]);
		edge* e = g->v[v].start->next;
		is force(&d->v[v], e->v < v ? e->v : low[e->v].a, &d->end) ok;
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
			is force(&d->v[v], elp[v]->v < v ? elp[v]->v : low[elp[v]->v].a, &d->end) ok;
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
				is force(&d2, d->v[v].l, &d->end) ok;
				is add(&d->v[v], &d2, &d->end) ok;
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

int counte(char* s, int len) {
	uint64_t* s2 = (uint64_t*)s;
	int e = 0;
	for (size_t i = 0; 8*i < len; i++) {
		uint64_t x = s2[i]-0x3f3f3f3f3f3f3f3full;
		while (x) {
			x &= x-1;
			e++;
		}
	}
	return e;
}

int getg(FILE* fin, graph* g, char* s, int n) {
	int len = (n*(n-1)/2+5)/6;
	assert(fgets(s, n*(n-1)/12+10, fin));
	if (counte(s, len) > 3*n-6) return 0;
	size_t i = 0, j = 0;
	for (size_t x = 0; ; x++) {
		char c = s[x]-0x3f;
		for (size_t y = 0; y < 6; y++) {
			if (i == j) {
				i++;
				if (i == n) return 1;
				j = 0;
			}
			if (c & 0x20) {
				pushg(g, i, j);
				pushg(g, j, i);
			}
			j++;
			c <<= 1;
		}
	}
}

int getn(FILE* fin) {
	char n = getc(fin);
	assert(n-63);
	return n - 63;
}

int main() {
	int n = getn(stdin);
	char* s = malloc(n*(n-1)/12+10);
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
	while (!feof(stdin)) {
		memset(g.v, 0, sizeof(vertex)*n);
		g.elen = 0;
		memset(s, 0, n*(n-1)/12+10);
		if (!getg(stdin, &g, s, n)) goto next;
		memset(g2.v, 0, sizeof(vertex)*n);
		g2.elen = 0;
		if (!dfs(&g, low, ord, &g2, elp)) goto next;
		sortg(&g2, low, ord);
		memset(dss.c, 0, sizeof(constraint)*(3*n-6));
		memset(dss.v, 0, sizeof(ds)*n);
		dss.end = 0;
		memset(elp, 0, sizeof(edge*)*n);
		if (planarity1(&g2, &dss, low, elp)) {
			printf("%c%s", n+63, s);
		}
next:
		if (feof(stdin)) break;
		getn(stdin);
	}
}
