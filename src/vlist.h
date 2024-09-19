

int getg(FILE* fin, graph* g, char* s, int n) {
	assert(fgets(s, n*(n-1)/12+3+7, fin));
	//handle first character
	char c = s[0]-0x3f;
	if (c & 0x20) {
		pushg(g, 0, 1);
		pushg(g, 1, 0);
	}
	if (c & 0x10) {
		pushg(g, 0, 2);
		pushg(g, 2, 0);
	}
	if (c & 0x08) {
		pushg(g, 1, 2);
		pushg(g, 2, 1);
	}
	if (c & 0x04) {
		pushg(g, 0, 3);
		pushg(g, 3, 0);
	}
	if (c & 0x02) {
		pushg(g, 1, 3);
		pushg(g, 3, 1);
	}
	if (c & 0x01) {
		pushg(g, 2, 3);
		pushg(g, 3, 2);
	}
	//handle rest
	size_t sci = 4, scj = 0;
	size_t emax = 6*n-12;
	for (size_t x = 1;; x++) {
		c = s[x]-0x3f;
		size_t prev = scj;
		scj %= sci;
		sci += prev >= sci;
		if (sci >= n) return g->elen;
		while (c) {
			if (g->elen >= emax) return emax+1;
			int o = 5-__builtin_ctz(c);
			c &= c-1;
			size_t i = sci + (scj+o >= sci);
			size_t j = (scj + o) % sci;
			pushg(g, i, j);
			pushg(g, j, i);
		}
		scj += 6;
	}
}

//abusing the parent field in g - "stackless"
//g2 is directed tree with set parents
int dfs(graph* g, dlow low[], size_t ord[], graph* g2, edge** elp) {
	for (size_t i = 0; i < g->n; i++) {
		low[i] = (dlow){i, i};
		elp[i] = g->v[i].start;
	}
	int top = -1;
	uint64_t nexp = (1ull << g->n) - 1;
	while (nexp) {
		size_t i = __builtin_ctzll(nexp);
		nexp &= nexp-1;
		size_t edges = 0;
		int otop = top;
		size_t v = i;
		ord[v] = ++top;
		g2->v[top].par = 0;
		while (1) {
			size_t v2;
			size_t t = ord[v];
			FORVC(v2, elp[v]) {
				size_t tx = ord[v2];
				if (1ull << v2 & ~nexp) {
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
					nexp ^= 1ull << v2;
					if (g->v[v2].start->next == NULL) {
						g2->n--;
						continue;
					}
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
	return 1;
}
