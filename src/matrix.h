#define MATRIX
#include <immintrin.h>

typedef uint64_t gmat;

int printmat(gmat* g, int n) {
	for (size_t i = 0; i < n; i++) {
		for (size_t j = 0; j < n; j++) {
			if (g[i] >> j & 0x1) {
				eprintf("██");
			}
			else {
				eprintf("  ");
			}
		}
		eprintf("\n");
	}
	eprintf("\n");
	return 0;
}

int printymm(__m256i x) {
	uint64_t a[4];
	_mm256_storeu_si256((__m256i*)a, x);
	for (size_t i = 0; i < 4; i++) {
		for (size_t j = 0; j < 64; j++) {
			printf("%li ", a[i] >> j & 0x1);
		}
		printf("\n");
	}
	printf("\n");
	return 0;
}

inline void magic2(gmat* g, size_t i, size_t j, const int sh, const uint64_t imm2) {
	__m256i x = _mm256_lddqu_si256((__m256i*)&g[i]);
	__m256i y = _mm256_lddqu_si256((__m256i*)&g[j]);
	__m256i m = _mm256_srli_epi64(x, sh);
	m = _mm256_xor_si256(y, m);
	__m256i m2 = _mm256_set1_epi64x(imm2);
	m = _mm256_and_si256(m2, m);
	y = _mm256_xor_si256(y, m);
	m = _mm256_slli_epi64(m, sh);
	x = _mm256_xor_si256(x, m);
	_mm256_storeu_si256((__m256i*)&g[i], x);
	_mm256_storeu_si256((__m256i*)&g[j], y);
}

inline uint8_t rev_byte2(uint8_t x) {	//reverse only lower 6 bits
	return (x * 0x80200802ULL & 0x0884422110ULL) * 0x0101010101ULL >> 34;
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

inline const size_t countev(uint64_t v) {
	size_t e = 0;
	while (v) {
		v &= v-1;
		e++;
	}
	return e;
}

int getmat(FILE* fin, gmat* g, char* s, int n) {
	assert(fgets(s, n*(n-1)/12+3, fin));
	s[(n*(n-1)/2+5)/6] = 0x3f;
	s[(n*(n-1)/2+5)/6+1] = 0x3f;
	int res = counte(s, n*(n-1)/12+3);
	s[(n*(n-1)/2+5)/6] = '\n';
	s[(n*(n-1)/2+5)/6+1] = 0;
	if (res > 3*n-6) return res;
	int x = 0;
	size_t o = 0;
	for (int i = 0; i < n; i++) {
		size_t j = 0;
		for (; j < i+o; j += 6) {
			g[i] |= (uint64_t)rev_byte2(s[x]-0x3f) << j >> o;
			x++;
		}
		g[i] &= (1ull << i)-1ull;
		o += i;
		o %= 6;
		x -= (o != 0);
	}
	gmat g2[64];
	memcpy(g2, g, 64*sizeof(gmat));
	for (size_t i = 0; i < 8; i++) {	//hopefully unroll, does only 8 words ATST
		__m256i a = _mm256_lddqu_si256((__m256i*)&g2[i*8]);
		__m256i b = _mm256_lddqu_si256((__m256i*)&g2[i*8+4]);
		__m256i x = _mm256_castpd_si256(_mm256_shuffle_pd(_mm256_castsi256_pd(a),_mm256_castsi256_pd(b),0x0));
		__m256i y = _mm256_castpd_si256(_mm256_shuffle_pd(_mm256_castsi256_pd(a),_mm256_castsi256_pd(b),0xf));
		__m256i m = _mm256_srli_epi64(x, 1);
		m = _mm256_xor_si256(y, m);
		__m256i m2 = _mm256_set1_epi64x(0x5555555555555555ull);
		m = _mm256_and_si256(m2, m);
		y = _mm256_xor_si256(y, m);
		m = _mm256_slli_epi64(m, 1);
		x = _mm256_xor_si256(x, m);
		a = _mm256_castpd_si256(_mm256_shuffle_pd(_mm256_castsi256_pd(x), _mm256_castsi256_pd(y), 0x0));
		b = _mm256_castpd_si256(_mm256_shuffle_pd(_mm256_castsi256_pd(x), _mm256_castsi256_pd(y), 0xf));
		_mm256_storeu_si256((__m256i*)&g2[i*8], a);
		_mm256_storeu_si256((__m256i*)&g2[i*8+4], b);
	}
	for (size_t i = 0; i < 8; i++) {	//hopefully unroll, does only 8 words ATST
		__m256i a = _mm256_lddqu_si256((__m256i*)&g2[i*8]);
		__m256i b = _mm256_lddqu_si256((__m256i*)&g2[i*8+4]);
		b = _mm256_permute4x64_epi64(b,0x4e);
		__m256i x = _mm256_castpd_si256(_mm256_blend_pd(_mm256_castsi256_pd(a),_mm256_castsi256_pd(b),0xc));
		__m256i y = _mm256_castpd_si256(_mm256_blend_pd(_mm256_castsi256_pd(a),_mm256_castsi256_pd(b),0x3));
		x = _mm256_permute4x64_epi64(x,0x4e);
		__m256i m = _mm256_srli_epi64(x, 2);
		m = _mm256_xor_si256(y, m);
		__m256i m2 = _mm256_set1_epi64x(0x3333333333333333ull);
		m = _mm256_and_si256(m2, m);
		y = _mm256_xor_si256(y, m);
		m = _mm256_slli_epi64(m, 2);
		x = _mm256_xor_si256(x, m);
		x = _mm256_permute4x64_epi64(x,0x4e);
		a = _mm256_castpd_si256(_mm256_blend_pd(_mm256_castsi256_pd(x),_mm256_castsi256_pd(y),0xc));
		b = _mm256_castpd_si256(_mm256_blend_pd(_mm256_castsi256_pd(x),_mm256_castsi256_pd(y),0x3));
		b = _mm256_permute4x64_epi64(b,0x4e);
		_mm256_storeu_si256((__m256i*)&g2[i*8], a);
		_mm256_storeu_si256((__m256i*)&g2[i*8+4], b);
	}
	for (size_t i = 0; i < 8; i++) {	//hopefully unroll, does only 8 words ATST
		magic2(g2, i*8, i*8+4, 4, 0x0f0f0f0f0f0f0f0full);
	}
	for (size_t i = 0; i < 4; i++) {	//hopefully unroll
		magic2(g2, i*16, i*16+8, 8, 0x00ff00ff00ff00ffull);
		magic2(g2, i*16+4, i*16+12, 8, 0x00ff00ff00ff00ffull);
	}
	for (size_t i = 0; i < 4; i++) {	//hopefully unroll
		magic2(g2, i*4, i*4+16, 16, 0x0000ffff0000ffffull);
		magic2(g2, i*4+32, i*4+48, 16, 0x0000ffff0000ffffull);
	}
	if (n > 32) for (size_t i = 0; i < 8; i++) {	//hopefully unroll
		magic2(g2, i*4, i*4+32, 32, 0x00000000ffffffffull);
	}
	for (size_t i = 0; i < n; i++) {
		g[i] |= g2[i];
	}
	//printmat(g, n);
	return res;
}

//recursive
int matdfs(gmat* g, int n, int e, dlow low[], size_t ord[], graph* g2) {
	int top = -1;
	gmat exp = (1ull << n)-1;
	int edges = 0;
	void dfs1(int vi) {
		gmat v = g[vi];
		int ti = ord[vi];
		g2->v[ti].start = &g2->e[g2->elen];
		g2->elen += __builtin_popcountll(v);
		for (gmat back = v & ~exp; back; back &= back-1) {
			int vi2 = __builtin_ctzll(back);
			int ti2 = ord[vi2];
			//add back edge
			if (ti2 < low[ti].b && ti2 != low[ti].a) {
				low[ti].b = MAX(ti2, low[ti].a);
				low[ti].a = MIN(ti2, low[ti].a);
			}
			if (g2->v[ti].par != ti2) pushg(g2, ti, ti2);
		}
		for (gmat forw = v; forw &= exp;) {
			int vi2 = __builtin_ctzll(forw);
			//add tree edge
			exp ^= 1ull << vi2;
			if (!(g[vi2] & g[vi2]-1)) {
				g2->n--;
				continue;
			}
			ord[vi2] = ++top;
			int ti2 = ord[vi2];
			g2->v[top].par = ti;
			pushg(g2, ti, top);
			dfs1(vi2);
			if (low[ti2].a < low[ti].b && low[ti].a != low[ti2].a) {
				low[ti].b = MAX(low[ti].a, low[ti2].a);
				low[ti].a = MIN(low[ti].a, low[ti2].a);
			}
			low[ti].b = MIN(low[ti2].b, low[ti].b);
		}
		return;
	}
	int add = 0;
	for (size_t i = 0; i < n; i++) {
		low[i] = (dlow){i, i};
	}
	while (exp) {
		int v0 = __builtin_ctzll(exp);
		exp ^= 1ull << v0;
		ord[v0] = ++top;
		g2->v[top].par = 0;
		if (top && g[v0]) {
			pushg(g2, top-1, top);
			add++;
		}
		if (!g[v0]) {
			top--;
			g2->n--;
			continue;
		}
		dfs1(v0);
		if (add+e > 3*n-6) return 0;
	}
	return 1;
}
