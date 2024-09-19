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

typedef struct {
	uint64_t v[64];
	size_t n;
} sgraph;

#define CONCI(A, B) A ## B
#define CONCAT(A, B) CONCI(A, B)
#define FORV(A, V) for (edge* CONCAT(next, __LINE__) = V.start; CONCAT(next, __LINE__) && ((A = CONCAT(next, __LINE__)->v) || 1); CONCAT(next, __LINE__) = CONCAT(next, __LINE__)->next) //predefine A
#define FORVC(A, E) for (;E && ((A = E->v) || 1); E = E->next) //prefine A and E
#define FORVP(A, V) for (edge* CONCAT(next, __LINE__) = V.start; CONCAT(next, __LINE__) && ((A = &CONCAT(next, __LINE__)->v) || 1); CONCAT(next, __LINE__) = CONCAT(next, __LINE__)->next)
#define suppose(X) if (!(X)) return 0

#define kk ?1:exit(__LINE__)
#define eprintf(...) fprintf(stderr, __VA_ARGS__)

inline void pushg(graph* g, size_t v, size_t v2) {
	g->e[g->elen] = (edge){g->v[v].start, v2};
	g->v[v].start = &g->e[g->elen++];
	return;
}

typedef struct {
	size_t a;
	size_t b;
} dlow;
#define MIN(A, B) ((A) < (B) ? (A) : (B))
#define MAX(A, B) ((A) > (B) ? (A) : (B))
