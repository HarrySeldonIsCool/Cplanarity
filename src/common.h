#include <sys/param.h>

typedef size_t edge;

typedef struct {
	edge* start;
	size_t len;
	size_t par;
} vertex;

typedef struct {
	edge* e;
	size_t elen;
	vertex* v;
	size_t n;
} graph;

/*#define CONCI(A, B) A ## B
#define CONCAT(A, B) CONCI(A, B)
#define FORV(A, V) for (edge* CONCAT(next, __LINE__) = V.start; CONCAT(next, __LINE__) && ((A = CONCAT(next, __LINE__)->v) || 1); CONCAT(next, __LINE__) = CONCAT(next, __LINE__)->next) //predefine A
#define FORVC(A, E) for (;E && ((A = E->v) || 1); E = E->next) //prefine A and E
#define FORVP(A, V) for (edge* CONCAT(next, __LINE__) = V.start; CONCAT(next, __LINE__) && ((A = &CONCAT(next, __LINE__)->v) || 1); CONCAT(next, __LINE__) = CONCAT(next, __LINE__)->next)*/
#define suppose(X) if (!(X)) return 0

#define eprintf(...) fprintf(stderr, __VA_ARGS__)

inline void pushg(graph* g, size_t v, size_t v2) {
	//assert(g->n-1 == v);
	
	g->v[v].start[g->v[v].len++] = v2;
	return;
}

typedef struct {
	size_t a;
	size_t b;
} dlow;
