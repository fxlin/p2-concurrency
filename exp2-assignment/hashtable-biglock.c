#include <glib.h>
#include <stdio.h>
#include <stdlib.h>

#include <time.h>
#include <string.h>
#include <assert.h>

#include <pthread.h>
#include <signal.h>
#include <errno.h>

#ifdef USE_VTUNE
#include <ittnotify.h>
#endif

#include "common.h"
#include "measure.h"

int numHashTables = 1;
GHashTable **hashtables;

int *keys;
int *vals;

int *spinLocks = NULL;
pthread_mutex_t *mutexes = NULL;

int the_n_elements = 0;

struct prog_config the_config;

long long runTime = 0;

#define ONE_BILLION 1000000000L;

#ifdef USE_VTUNE
__itt_domain *itt_domain = NULL;
__itt_string_handle *sh_sort = NULL;   // the overall task name
__itt_string_handle **sh_parts = NULL; // per part task name

#define vtune_task_begin(X) __itt_task_begin(itt_domain, __itt_null, __itt_null, sh_parts[X])
#define vtune_task_end() __itt_task_end(itt_domain)
#else
#define vtune_task_begin(X)
#define vtune_task_end()
#endif

void print_errors(char *error)
{
	if (strcmp(error, "clock_gettime") == 0) {
		fprintf(stderr, "Error initializing clock time\n");
		exit(1);
	}
	if (strcmp(error, "thread_create") == 0) {
		fprintf(stderr, "Error creating threads.\n");
		exit(2);
	}
	if (strcmp(error, "thread_join") == 0) {
		fprintf(stderr, "Error with pthread_join.\n");
		exit(2);
	}
	if (strcmp(error, "mutex") == 0) {
		fprintf(stderr, "Error with pthread_join. \n");
		exit(2);
	}
	if (strcmp(error, "segfault") == 0) {
		fprintf(stderr, "Segmentation fault caught! \n");
		exit(2);
	}
	if (strcmp(error, "size") == 0) {
		fprintf(stderr, "Sorted List length is not zero. List Corrupted\n");
		exit(2);
	}
	if (strcmp(error, "lookup") == 0) {
		fprintf(stderr, "Could not retrieve inserted element due to corrupted list.\n");
		exit(2);
	}
	if (strcmp(error, "length") == 0) {
		fprintf(stderr, "Could not retrieve length because list is corrupted.\n");
		exit(2);
	}
	if (strcmp(error, "delete") == 0) {
		fprintf(stderr, "Could not delete due to corrupted list. \n");
		exit(2);
	}
}

void signal_handler(int sigNum)
{
	if (sigNum == SIGSEGV)
		print_errors("segfault");
}

// Generate keys to insert into the hash table.
int *alloc_keys(int numElements)
{
	fprintf(stderr, "init %d keys\n", numElements);

	int *keys = malloc(sizeof(int) * numElements);
	assert(keys);
	for (int i = 0; i < numElements; i++)
		keys[i] = rand();
	return keys;
}

// Generate values to insert into the hash table.
int *alloc_vals(int numElements)
{
	fprintf(stderr, "init %d vals\n", numElements);

	int *vals = malloc(sizeof(int) * numElements);
	assert(vals);
	for (int i = 0; i < numElements; i++)
		vals[i] = rand();
	return vals;
}

// The H(K) function. mapping a given key to a given hashtable
// @key: the key; return: the index of the hashtable. 
int find_which_hashtable(int key)
{
	/* TODO: overwrite everything here to define your own H(K) */
	key = key; 	
	return 0; 
}

GHashTable **alloc_hashtables(int n_hashtables)
{
	assert(n_hashtables > 0);
	GHashTable **hashtables = malloc(sizeof(GHashTable *) * n_hashtables);
	assert(hashtables);
	for (int i = 0; i < n_hashtables; i++)
		hashtables[i] = g_hash_table_new_full(g_direct_hash, g_direct_equal, NULL, NULL);
	printf("allocated %d GHashTables\n", n_hashtables);
	return hashtables;
}

// The function each worker thread runs.
// Inserts all data into the hash table.
void *thread_func(void *thread_id)
{
	int id = *((int *)thread_id);

	pthread_mutex_lock(&mutexes[0]);	// must be locked to prevent trace corruption
	k2_measure("tr start");	// for simplicity, the trace does not include thread id 
	pthread_mutex_unlock(&mutexes[0]);

	int per_part = the_n_elements / the_config.numParts; // Num of keys this worker needs to insert

	vtune_task_begin(id);
	// Insert a range of (key, val) into the hash table. The range depends on @id (thread_id)
	for (int i = per_part * id; i < per_part * (id + 1); i++)
	{		
		// The current code only uses a single hash table and a lock.
		// TODO: to scale it up, rewrite/modify the code so it selects a hash table to insert into 
		// 	and insert the data
		assert(id >= 0);
		pthread_mutex_lock(&mutexes[0]);
		g_hash_table_insert(hashtables[0], &keys[i], &vals[i]);
		pthread_mutex_unlock(&mutexes[0]);
	}
	vtune_task_end();

	pthread_mutex_lock(&mutexes[0]);
	k2_measure("tr done");
	pthread_mutex_unlock(&mutexes[0]);

	return NULL;
}

int main(int argc, char **argv)
{
	the_config = parse_config(argc, argv);

	int numThreads = the_config.numThreads;
	int iterations = the_config.iterations;
	int numParts = the_config.numParts;

	// The current code only uses 1 hashtable & 1 lock. 
	// TODO: ensure N hashtables and N locks are allocated & used
	numHashTables = 1;

	printf("thread count: %d\niters: %d\nhash tables: %d\n", numThreads, iterations, numHashTables);

	signal(SIGSEGV, signal_handler);

	k2_measure("init");

	srand((unsigned int)time(NULL));

	the_n_elements = numThreads * iterations;

	// Allocate lock(s). 
	alloc_locks(&mutexes, numHashTables, NULL, 0);
	// Allocate hash table(s).
	hashtables = alloc_hashtables(numHashTables);

	// Generate keys and values to be inserted into the hash table(s). 
	keys = alloc_keys(the_n_elements);
	vals = alloc_vals(the_n_elements);

#ifdef USE_VTUNE
	itt_domain = __itt_domain_create("my domain");
	__itt_thread_set_name("my main");

	// pre create task_name, instead of doing it inside tasks
	sh_parts = malloc(sizeof(__itt_string_handle *) * numParts);
	assert(sh_parts);
	char itt_task_name[32];
	for (int i = 0; i < numParts; i++)
	{
		snprintf(itt_task_name, 32, "part-%d", i);
		sh_parts[i] = __itt_string_handle_create(itt_task_name);
	}
#endif

	k2_measure("init done");

	pthread_t threads[numThreads];
	int thread_id[numThreads];

	struct timespec start, end;
	if (clock_gettime(CLOCK_MONOTONIC, &start) < 0)
		print_errors("clock_gettime");

#ifdef USE_VTUNE
	__itt_task_begin(itt_domain, __itt_null, __itt_null,
					 __itt_string_handle_create("hashtable"));
#endif

	for (int i = 0; i < numThreads; i++)
	{
		thread_id[i] = i;
		int rc = pthread_create(&threads[i], NULL, thread_func, &thread_id[i]);
		if (rc < 0)
			print_errors("thread_create");
	}
	k2_measure("tr launched");

	for (int i = 0; i < numThreads; i++)
	{
		int rc = pthread_join(threads[i], NULL);
		if (rc < 0)
			print_errors("thread_join");
	}
	printf("all threads finished, check correctness..\n");
#ifdef USE_VTUNE
	__itt_task_end(itt_domain);
	free(sh_parts);
#endif

	k2_measure("tr joined");

	if (clock_gettime(CLOCK_MONOTONIC, &end) < 0)
		print_errors("clock_gettime");

	long long diff = (end.tv_sec - start.tv_sec) * ONE_BILLION;
	diff += end.tv_nsec;
	diff -= start.tv_nsec;

	// we're done. correctness check up
	{
		// make sure all keys are inserted and values are correct
		for (int i = 0; i < the_n_elements; i++)
		{
			int index = find_which_hashtable(keys[i]);
			assert(index >= 0);
			gpointer rc = g_hash_table_lookup(hashtables[index], &keys[i]);
			int *val_in_table = rc;
			assert(*val_in_table == vals[i]);
		}
	}
	k2_measure_flush();


	int numOpts = iterations * numThreads; // get number of operations

	char testname[32] = "hashtable";
	print_csv_line(testname, numThreads, iterations, numParts, numOpts, diff);

	// --- clean up ---- //
	free_locks(mutexes, numHashTables, spinLocks);
	free(keys);
	free(vals);

	exit(0);
}
