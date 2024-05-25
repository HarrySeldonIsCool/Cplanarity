#define eprintf(...) fprintf(stderr, __VA_ARGS__)
#define MIN(A, B) ((A) < (B) ? (A) : (B))
#define MAX(A, B) ((A) > (B) ? (A) : (B))
typedef struct {
	uint64_t e;
	size_t l;
} eset;

typedef struct {
	eset l;
	eset r;
} constraint;

typedef struct {
	constraint* start;
	size_t len;
	size_t l;
} ds;

typedef struct {
	constraint* c;
	ds* v;
	size_t end;
} dds;

int force(ds* d, size_t low, size_t* end) {
	uint64_t mask = low+1 > 64 ? 0 : ~0ull << low+1;
	size_t off = 0;
	constraint acc = {(eset){0, 69}, (eset){0, 69}};
	for (size_t i = 0; i < d->len; i++) {
		eset l = d->start[i].l;
		eset r = d->start[i].r;
		switch (((l.e&mask)!=0)+2*((r.e&mask)!=0)) {
			case 0:
				d->start[i-off] = d->start[i];
				break;
			case 2:
				eset c = l;
				l = r;
				r = c;
			case 1:
				acc.l = (eset){acc.l.e|l.e, MIN(acc.l.l, l.l)};
				acc.r = (eset){acc.r.e|r.e, MIN(acc.r.l, r.l)};
				off++;
				break;
			case 3:
				return 0;
		}
	}
	d->len -= off;
	*end -= off;
	if (acc.l.e | acc.r.e) {
		d->start[d->len++] = acc;
		(*end)++;
	}
	return 1;
}

int add(ds* d, ds* d2, size_t* end) {
#define fops(a, b) (b.l+1 < 64 ? a.e >> b.l+1 : 0)
#define comp(a, b) (!fops(a, b) || !fops(b, a))
	size_t nlen = 0;
	for (size_t i = 0; i < d2->len; i++) {
		/*if ((d2->start[i].l.e | d2->start[i].r.e) >> d->l+1 == 0 || d->l+1 > 64) {
			d->start[d->len+(nlen++)] = d2->start[i];
			continue;
		}*/
		size_t off = 0;
		constraint acc = d2->start[i];
		for (size_t j = 0; j < d->len; j++) {
			eset l = d->start[j].l;
			eset r = d->start[j].r;
			int sim = comp(d2->start[i].l, l) && comp(d2->start[i].r, r);
			int opp = comp(d2->start[i].l, r) && comp(d2->start[i].r, l);
			switch (2*sim + opp) {
				case 0:
					return 0;
				case 1:
					eset c = l;
					l = r;
					r = c;
				case 2:
					acc.l = (eset){acc.l.e|l.e, MIN(acc.l.l, l.l)};
					acc.r = (eset){acc.r.e|r.e, MIN(acc.r.l, r.l)};
					off++;
					break;
				case 3:
					d->start[j-off] = d->start[j];
					break;
			}
		}
		for (size_t j = 0; j < nlen; j++) {
			d->start[j+d->len-off] = d->start[j+d->len];
		}
		d->len -= off;
		*end -= off;
		d->start[d->len+(nlen++)] = acc;
	}
	d->len += nlen;
	d->l = MIN(d->l, d2->l);
	return 1;
#undef fops
#undef comp
}

void limit(ds* d, size_t x, size_t* end) {
	uint64_t mask = (1ull<<x)-1;
	size_t off = 0;
	for (size_t i = 0; i < d->len; i++) {
		d->start[i].r.e &= mask;
		d->start[i].l.e &= mask;
		if (d->start[i].r.e) {
			d->start[i].r.l = d->l;
		}
		if (d->start[i].l.e) {
			d->start[i].l.l = d->l;
		}
		if (d->start[i].l.e | d->start[i].r.e) {
			d->start[i-off] = d->start[i];
		}
		else {
			off++;
		}
	}
	*end -= off;
	d->len -= off;
}
