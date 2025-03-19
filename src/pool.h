#include <pthread.h>
#include <assert.h>

typedef struct {
	void* it;
	pthread_mutex_t lock;
	int stage;
} pool_item;

typedef struct {
	int stages;
	pool_item* its;
	int size;
	int* free;		//[stages]
	pthread_cond_t* fc;	//[stages]
	pthread_mutex_t main;	//neccessary because im bad
	int operational;
	uint64_t active;
} pool;

//Example usage:
//void worker(pool* p) {
//	const int stage = 2; //for example
//	while (active) {
//		pool_item* it = get_item(p, stage);
//		DO_STUFF(it->it);
//		release_item(p, it);
//	}
//}

//if you start using it before init you're a dumbass
void init_pool(pool* p, int stages, int size, void (*init_item)(void**)) {
	assert(stages < 64);
	p->main = (pthread_mutex_t)PTHREAD_MUTEX_INITIALIZER;
	pthread_mutex_lock(&p->main);
	p->its = malloc(size*sizeof(pool_item));
	p->size = size;
	p->stages = stages;
	for (size_t i = 0; i < size; i++) {
		p->its[i].lock = (pthread_mutex_t)PTHREAD_MUTEX_INITIALIZER;
		p->its[i].stage = 0;
		init_item(&p->its[i].it);
	}
	p->free = malloc(stages*sizeof(int));
	p->free[0] = size;
	p->fc = malloc(stages*sizeof(pthread_cond_t));
	p->operational = size;
	p->active = (1ull << stages) - 1;
	pthread_mutex_unlock(&p->main);
	return;
}

//if you call before the threads are done with it, you're dumb
void destroy_pool(pool* p, void (*destroy_item)(void*)) {
	pthread_mutex_lock(&p->main);
	int sum = 0;
	for (size_t i = 0; i < p->stages; i++) {
		sum += p->free[i];
	}
	assert(sum == p->size);	//you're dumb
	free(p->free);
	free(p->fc);
	for (size_t i = 0; i < p->size; i++) {
		destroy_item(p->its[i].it);
		pthread_mutex_destroy(&p->its[i].lock);
	}
	free(p->its);
	pthread_mutex_unlock(&p->main);
	pthread_mutex_destroy(&p->main);
	return;
}

//on all items finished returns NULL
//auto-increments stage
pool_item* get_item(pool* p, int stage) {
	pthread_mutex_lock(&p->main);
	while (p->free[stage] == 0 && p->operational) {
		pthread_cond_wait(&p->fc[stage], &p->main);
	}
	if (!p->operational) {
		pthread_mutex_unlock(&p->main);
		return NULL;
	}
	for (size_t i = 0; i < p->size; i++) {
		if (p->its[i].stage != stage) {
			continue;
		}
		if (pthread_mutex_trylock(&p->its[i].lock)) {
			continue;
		}
		p->free[stage]--;
		p->its[i].stage++;
		p->its[i].stage %= p->stages;
		pthread_mutex_unlock(&p->main);
		return &p->its[i];
	}
	return NULL;	//unreachable
}

void release_item(pool* p, pool_item* pi) {
	pthread_mutex_lock(&p->main);
	int stage = pi->stage;
	pthread_mutex_unlock(&pi->lock);
	p->free[stage]++;
	if ((p->active >> stage & 1ull) == 0) {
		p->operational--;
		if (!p->operational) {
			for (size_t i = 0; i < p->stages; i++) {
				pthread_cond_broadcast(&p->fc[i]);
			}
		}
	} else {
		pthread_cond_signal(&p->fc[stage]);
	}
	pthread_mutex_unlock(&p->main);
	return;
}

//between get and release
void set_stage(pool* p, pool_item* pi, int stage) {
	pthread_mutex_lock(&p->main);
	pi->stage = stage;
	pthread_mutex_unlock(&p->main);
}

void kill_stage(pool* p, int stage) {
	pthread_mutex_lock(&p->main);
	p->active ^= 1ull << stage;
	p->operational -= p->free[stage];
	pthread_mutex_unlock(&p->main);
}
