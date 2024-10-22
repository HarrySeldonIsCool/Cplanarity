//works only for E < 64 => N < 23
//>w<

typedef struct sehedge {
	uint64_t e;
	uint64_t low1;
	uint64_t low2;
	struct sehedge* next;
} ehedge;

typedef struct {
	ehedge* start;
} ehvertex;

typedef struct {
	ehedge* e;	//buffer - 2*n^2, constraints as marked hyperedges
	ehvertex* v;	//edges in original
	size_t elen;
	int n;		//num edges in original graph
} ehgraph;

void pushehg(ehgraph* g, size_t v, uint64_t v2, uint64_t low1, uint64_t low2) {
	g->e[g->elen] = (ehedge) {
		v2,
		low1,
		low2,
		g->v[v].start
	};
	g->v[v].start = &g->e[g->elen++];
	return;
}


//don't need to check for colors, just dfs the shortest path
//burnt ground approach (leaves only The Path in the graph)
//TODO switch to bfs (probably faster most of the time)
size_t complete_cycle(ehgraph* g, size_t start, size_t end, size_t max_depth, uint64_t* exp) {
	if (max_depth == 0) return 1;
	*exp |= 1ull << start;
	for (ehedge* e = g->v[start].start; e; e = e->next) {
		if (e->e & 1ull << end) {
			g->v[start].start = e;
			e->e = 1ull << end;
			return 1;
		}
		if (max_depth == 1) return 2;
		uint64_t mask = e->e;
		while (mask = e->e & ~*exp) {
			size_t v = __builtin_ctzll(mask);
			size_t l = findcycle(g, v, end, max_depth-1, exp);
			if (l < max_depth) {
				max_depth = l;
				g->v[start].start = e;
				e->e &= mask;
				if (l == 1) return 2; 
			}
		}
		e->e &= -e->e;
	}
	g->v[start].start->next = NULL;
	return max_depth+1;
}

//produce a set of all edges present in the cycle
uint64_t all_cycle_edges(ehgraph* g, size_t v) {
	if (g->v[v].start) {
		return 1ull << v | all_cycle_edges(g, __builtin_ctzll(g->v[v].start->e));
	}
	else {
		return 1ull << v;
	}
}

//only a critical, not minimal critical graph
uint64_t elim_lows(ehgraph* g, size_t v, uint64_t cycle) {
	ehedge* e = g->v[v].start;
	if (g->v[v].start) {
		cycle |= elim_lows(g, __builtin_ctzll(e->e), cycle);
	}
	if (~cycle & e->low1) {
		cycle |= e->low1 & -e->low1;
	}
	if (~cycle & e->low2) {
		cycle |= e->low2 & -e->low2;
	}
	return cycle;
}

//do not include the last constraint into g
//e1 and e2 are crashing edges (arbitrary)
//low1/2 are the sets of lowpoints forcing the crash
//returns set of co-tree critical edges
void critical_set(ehgraph* g, size_t e1, size_t e2, uint64_t low1, uint64_t low2) {
	uint64_t exp = 0;
	g->v[end].start = NULL
	complete_cycle(g, e1, e2, -1, &exp);
	uint64_t e = all_cycle_edges(g, e1);
	e = elim_lows(g, e1, e);
	if (~e & low1) {
		e |= low1 & -low1;
	}
	if (~e & low2) {
		e |= low2 & -low2;
	}
	return e;
}

void elim_others(graph* g, uint64_t keep, size_t v) {
	for (edge** e = &g->v[v].start; *e;) {
		if ((*e)->v > v) {
			elim_others(g, keep, (*e)->v);
		}
		else {
			if (~keep & 1ull << (*e - g->e) / sizeof(edge)) {
				*e = (*e)->next;
			}
			else {
				e = &(*e)->next;
			}
		}
	}
}

int elim_dead(graph* g, size_t v) {
	for (edge** e = &g->v[v].start; *e;) {
		if ((*e)->v > v) {
			switch (elim_dead(g, (*e)->v)) {
				case 1:
					*e = (*e)->next;
					break;
				case 2:
					*e = g->v[(*e)->v].start;
					break;
				case 0:
					e = &(*e)->next;
					break;
			}
		}
		else {
			e = &(*e)->next;
		}
	}
	if (!g->v[v].start) {
		return 1;
	}
	if (!g->v[v].start->next) {
		return 2;
	}
	return 0;
}

//make a graph co-tree critical (3-connected)
//TODO: add labels to edges (to reconstruct the subgraph)
void to_critical(graph* g, ehgraph* cg, size_t e1, size_t e2, uin64_t low1, uint64_t low2) {
	uint64_t e = critical_set(cg, e1, e2, low1, low2);
	elim_others(g, e, 0);
	elim_dead(g, 0);
}
