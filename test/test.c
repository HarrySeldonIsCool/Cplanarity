#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
//#include "common.h"
//#include "matrix.h"
//#include "vlist.h"
//#include "buckets.h"
//#include "printing.h"
#include "pool.h"
/*
int cmp_vlist_mat(gmat *g, graph* g2, int *(int*)it->itn) {
	for (size_t i = 0; i < n; i++) {
		gmat x = g[i];
		int v2;
		FORV(v2, g2->v[i]) {
			suppose(x & 1ull << v2);
			x ^= 1ull << v2;
		}
		suppose(!x);
	}
	return 1;
}

void test_parse() {
	//0 1 0 0 1
	//1 0 0 1 0
	//0 0 0 1 0
	//0 1 1 0 0
	//1 0 0 0 0
	char s[] = {0x3f+0b100011,0x3f+0b100000,'\n',0, 0x3f,0x3f,0x3f,0x3f,0x3f,0x3f,0x3f,0x3f};
	FILE* t = fopen(".test_parse", "w");
	fprintf(t, "%s", s);
	gmat g[64] = {0};
	fclose(t);
	t = fopen(".test_parse", "r");
	getmat(t, g, s, 5);
	gmat g2[] = {0b10010, 0b01001, 0b01000, 0b00110, 0b00001};
	assert(!memcmp(g, g2, sizeof(uint64_t)*5));
	return;
}

void test_ctz() {
	for (size_t i = 0; i < 64; i++) {
		assert(__builtin_ctzll(1ull << i) == i);
	}
}

void test_dfs() {
	int n = 40;
	char s[] = "????C???S?G??A??A?@?????A??A????A?G??_?C@????O??A??G??B?_????????CO?CO_Gc????a?C?C??CG?@C??__????a????????_C??CS??????_E???G????G?\n\0????????";
	FILE* t = fopen(".test_dfs", "w");
	fprintf(t, "%s", s);
	fprintf(t, "%s", s);
	gmat g[64] = {0};
	graph g2 = {
		calloc(6*40-12, sizeof(edge)),
		0,
		calloc(40, sizeof(vertex)),
		40
	};
	graph g3 = {
		calloc(3*40-6, sizeof(edge)),
		0,
		calloc(40, sizeof(vertex)),
		40
	};
	graph g4 = {
		calloc(3*40-6, sizeof(edge)),
		0,
		calloc(40, sizeof(vertex)),
		40
	};
	dlow* low = calloc(n,sizeof(dlow));
	size_t* ord = calloc(n,sizeof(size_t));
	edge** elp = calloc(n,sizeof(edge*));
	fclose(t);
	t = fopen(".test_dfs", "r");
	int e = getmat(t, g, s, 40);
	assert(e*2 == getg(t, &g2, s, 40));
	printmat(g, 40);
	printg(stdout, &g2);
	assert(cmp_vlist_mat(g, &g2, 40));
	matdfs(g, n, e, low, ord, &g3);
	dfs(&g2, low, ord, &g4, elp);
	printf("\nmatrix:\n");
	printg(stdout, &g3);
	printf("\nvlist:\n");
	printg(stdout, &g4);
}
*/
void init_item(void** x) {
	*x = malloc(sizeof(int));
	*(int*)*x = 0;
	return;
}

void destroy_item(void* x) {
	free(x);
}

void* add2(void* pv) {
	pool* p = pv;
	const int stage = 0;
	for (size_t i = 0; i < 6; i++) {
		pool_item* it = get_item(p, stage);
		if (*(int*)it->it >= 0) {
			*(int*)it->it += 2;
		}
		release_item(p, it);
	}
	kill_stage(p, stage);
	return NULL;
}

void* sub1(void* pv) {
	pool* p = pv;
	const int stage = 1;
	while (p->operational) {
		pool_item* it = get_item(p, stage);
		if (it == NULL) continue;
		*(int*)it->it -= 3;
		release_item(p, it);
	}
	return NULL;
}

void* print(void* pv) {
	pool* p = pv;
	const int stage = 2;
	while (p->operational) {
		pool_item* it = get_item(p, stage);
		if (it == NULL) continue;
		printf("%i\n", *(int*)it->it);
		release_item(p, it);
	}
	return NULL;
}

void pool_test() {
	pool p;
	init_pool(&p, 3, 3, init_item);
	pthread_t ad;
	pthread_t su;
	pthread_t pr;
	pthread_create(&ad, NULL, add2, &p);
	pthread_create(&su, NULL, sub1, &p);
	pthread_create(&pr, NULL, print, &p);
	pthread_join(ad, NULL);
	pthread_join(su, NULL);
	pthread_join(pr, NULL);
	destroy_pool(&p, destroy_item);
	return;
}

int main() {
	/*test_parse();
	test_ctz();
	test_dfs();*/
	pool_test();
	printf("success!\n");
}
