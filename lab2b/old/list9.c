/*
 * biglock				no
 * malloc					no
 * straggler 			no
 * false sharing 	no
 */

// lab2_list-steal2-naivepadding
// many parts. load balancing. padding is naive: using 1024x listheads.
// see i5-padding-quirks.txt

#include <stdio.h>
#include <stdlib.h>

#include <time.h>
#include <string.h>
#include <assert.h>

#include "SortedList.h"
#include <pthread.h>
#include <signal.h>
#include <errno.h>

#include "common.h"  // xzl
#include "measure.h"  // xzl

//int iterations = 1;
//int numThreads = 1;
//int numList = 1;
//int numparts;
//int mutexFlag = 0;
//int spinLockFlag = 0;


SortedList_t* lists;
SortedListElement_t* elements;

int* spinLocks = 0;
pthread_mutex_t* mutexes = 0;

int numElements = 0;

struct prog_config the_config;

long long runTime = 0;

#define ONE_BILLION 1000000000L;


#if 0
void getTestName(){
	const char * sync;

    if (mutexFlag)
    	sync = "m";
    else if(spinLockFlag)
    	sync = "s";
    else
    	sync = "none";

    if (numList == 1)
    	snprintf(test, 32, "list-%s", sync);
    else
    	snprintf(test, 32, "list-%s-%dpart", sync, numList);
}
#endif

void print_errors(char* error){
    if(strcmp(error, "clock_gettime") == 0){
        fprintf(stderr, "Error initializing clock time\n");
        exit(1);
    }
    if(strcmp(error, "thread_create") == 0){
        fprintf(stderr, "Error creating threads.\n");
        exit(2);
    }
    if(strcmp(error, "thread_join") == 0){
        fprintf(stderr, "Error with pthread_join.\n");
        exit(2);
    }
    if(strcmp(error, "mutex") == 0){
        fprintf(stderr, "Error with pthread_join. \n");
        exit(2);
    }
    if(strcmp(error, "segfault") == 0){
        fprintf(stderr, "Segmentation fault caught! \n");
        exit(2);
    }
    if(strcmp(error, "size") == 0){
        fprintf(stderr, "Sorted List length is not zero. List Corrupted\n");
        exit(2);
    }
    if(strcmp(error, "lookup") == 0){
        fprintf(stderr, "Could not retrieve inserted element due to corrupted list.\n");
        exit(2);
    }
    if(strcmp(error, "length") == 0){
        fprintf(stderr, "Could not retrieve length because list is corrupted.\n");
        exit(2);
    }
    if(strcmp(error, "delete") == 0){
        fprintf(stderr, "Could not delete due to corrupted list. \n");
        exit(2);
    }
}

void signal_handler(int sigNum){
    if(sigNum == SIGSEGV){
        print_errors("segfault");
    }
}//signal handler for SIGSEGV

#if 0
// xzl: this only gens 1 char key? bad
//https://stackoverflow.com/questions/19724346/generate-random-characters-in-c generating random characters in C
char* getRandomKeyStr(){
    char* random_key = (char*) malloc(sizeof(char)*2);
    random_key[0] = (char) rand()%26 + 'a';
    random_key[1] = '\0';
    return random_key;
}

my_key_t getRandomKey() {
	my_key_t ret = 0;
	// per byte
	for (unsigned int i = 0; i < sizeof(ret); i++) {
		ret  |= rand() & 0xff;
		ret <<= 8 ;
	}
	return ret;
}
#endif

void setupLocks(){

    mutexes = malloc(sizeof(pthread_mutex_t)*the_config.numThreads*1024);

    for(int i = 0; i < the_config.numThreads * 1024; i++){
				if(pthread_mutex_init(&mutexes[i], NULL) < 0){
						print_errors("mutex");
				}
    }

    spinLocks = malloc(sizeof(int)*the_config.numParts);
    for (int i = 0; i < the_config.numParts; i++)
    	spinLocks[i] = 0;

    printf("init %d mutex\n", the_config.numThreads);
}

void CleanLocks() {

  if (mutexes) {
		for(int i = 0; i < the_config.numThreads*1024; i++)
				pthread_mutex_destroy(&mutexes[i]);
		free(mutexes);
  }

  if (spinLocks)
  	free(spinLocks);

}

// the resultant sublists
void initializeSubLists(){
    lists = malloc(sizeof(SortedList_t) * the_config.numParts * 1024); // padding
    for (int i = 0; i < the_config.numThreads*1024; i++){
        lists[i].prev = &lists[i];
        lists[i].next = &lists[i];
        lists[i].key = 0;
    }

    printf("init %d sub lists. sizeof(SortedList_t) = %lu\n",
    		the_config.numParts, sizeof(SortedList_t));
}

void initializeElements(int numElements){
    int i;
    fprintf(stderr, "init %d elements", numElements);

    for(i = 0; i < numElements; i++){
        elements[i].key = getRandomKey();
        // debugging
//        if (i < 10)
//        	printf("random keys: %lx \n", elements[i].key);
    }
}

void initializeLists() {
    initializeSubLists();
    setupLocks();
    initializeElements(numElements);
//    splitElements();
}

void* sorted_list_action(void* thread_id){
//    struct timespec start_time, end_time;
//    int i ,j;
    int num = *((int*)thread_id);

    pthread_mutex_lock(&mutexes[0]);
    k2_measure("tr start");
    pthread_mutex_unlock(&mutexes[0]);

//    printf("i'm thread %d\n", num);

    int per_part = numElements / the_config.numParts;

    for (int part = 0; part < the_config.numParts; part++) {
    	/* try to steal */
    	if (__sync_lock_test_and_set(&spinLocks[part], 1) == 0) {

//    		printf("thread %d got part %d \n", num, part);

    			for (int i = per_part * part; i < per_part * (part + 1); i++) {

						if(the_config.mutexFlag)
								pthread_mutex_lock(&mutexes[num*1024]);

							SortedList_insert(&lists[num*1024], &elements[i]);
//						SortedList_insert(&lists[num], &elements[i]);

						if(the_config.mutexFlag)
								pthread_mutex_unlock(&mutexes[num*1024]);
					}
    	}
    }

    pthread_mutex_lock(&mutexes[0]);
    k2_measure("tr done");
    pthread_mutex_unlock(&mutexes[0]);

    return NULL;
}

int main(int argc, char** argv) {
//    char input;

    the_config = parse_config(argc, argv);

		int numThreads = the_config.numThreads;
		int iterations = the_config.iterations;
		int numParts = the_config.numParts;

    signal(SIGSEGV, signal_handler);

    k2_measure("init");

    numElements = numThreads * iterations;
    elements = (SortedListElement_t*) malloc(sizeof(SortedListElement_t)*numElements);
    srand((unsigned int) time(NULL)); //must use srand before rand
    initializeLists();
    
    k2_measure("init done");

    pthread_t threads[numThreads];
    int thread_id[numThreads];

    struct timespec start, end;
    if (clock_gettime(CLOCK_MONOTONIC, &start) < 0){
        print_errors("clock_gettime");
    }
    
    for(int i = 0; i < numThreads; i++){
        thread_id[i] = i;
        int rc = pthread_create(&threads[i], NULL, sorted_list_action, &thread_id[i]);
        if(rc < 0){
            print_errors("thread_create");
        }
    }

    k2_measure("tr launched");

    for(int i = 0; i < numThreads; i++){
        int rc = pthread_join(threads[i], NULL);
        if(rc < 0){
            print_errors("thread_join");
        }
    }
    
    k2_measure("tr joined");

    if(clock_gettime(CLOCK_MONOTONIC, &end) < 0){
        print_errors("clock_gettime");
    }
    
    long long diff = (end.tv_sec - start.tv_sec) * ONE_BILLION;
    diff += end.tv_nsec;
    diff -= start.tv_nsec; //get run time
    
    // we're done. correctness check up
    {
    	long total = 0;
			for(int i = 0; i < numThreads * 1024; i+=1024){
					int ll = SortedList_length(&lists[i]);
					fprintf(stderr, "list %d: %d items; ", i, ll);
					total += ll;
			}
			fprintf(stderr, "\ntotal %ld items\n", total);
    }

    k2_measure_flush();

    int numOpts = iterations * numThreads; //get number of operations

    char testname[32];
    getTestName(&the_config, testname, 32);
    
    print_csv_line(testname, numThreads, iterations, numParts, numOpts, diff);

    // --- clean up ---- //

    // no need to free
#if 0
    for(i = 0; i < numElements; i++){
        free((void*)elements[i].key);
    }
#endif

    free(elements);
    free(lists);


    exit(0);
}



