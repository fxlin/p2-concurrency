#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <assert.h>

#include "common.h"

void getTestName(struct prog_config *config, char *buf, int buflen){
	const char * sync;

	assert(config && buf);

    if (config->mutexFlag)
    	sync = "m";
    else
    	sync = "none";

    if (config->numParts == 1)
    	snprintf(buf, buflen, "list-%s", sync);
    else
    	snprintf(buf, buflen, "list-%s-%dpart", sync, config->numParts);
}


//void print_csv_line(char* test, int threadNum, int iterations, int numList, int numOperation, long long runTime, long long avgTime, long long avgMutexTime){
    //fprintf(stdout, "%s,%d,%d,%d,%d,%lld,%lld,%lld\n", test, threadNum, iterations, numList, numOperation, runTime, avgTime, avgMutexTime);
//    fprintf(stdout, "test=%s threadNum=%d iterations=%d numList=%d numOperation=%d runTime(ns)=%lld avgTime(ns)=%lld avgMutexTime(ns)=%lld\n",
//    			test, threadNum, iterations, numList, numOperation, runTime, avgTime, avgMutexTime);

void print_csv_line(char* test, int threadNum, int iterations, int numList, int numOperation, long long runTime) {
		fprintf(stdout, "test=%s threadNum=%d iterations=%d numList=%d numOperation=%d runTime(ms)=%lld tput(Mops)=%.2f\n",
    			    			test, threadNum, iterations, numList, numOperation, runTime/(1000*1000), 1000 * (double)numOperation/runTime);
}

void usage() {
	fprintf(stderr, "./[this_program] --iterations iterations "
			"--threads numThreads=1 [--sync option] \n");
}

void alloc_locks(pthread_mutex_t **mutexes, int n_mutex,
		int **spinlocks, int n_spinlocks){

		if (mutexes && n_mutex) {
			*mutexes = malloc(sizeof(pthread_mutex_t) * n_mutex);
			assert(*mutexes);

			for(int i = 0; i < n_mutex; i++) {
					if(pthread_mutex_init((*mutexes) + i, NULL) < 0){
							perror("mutex");
							exit(1);
					}
			}
		}

		if (spinlocks && n_spinlocks) {
			*spinlocks = malloc(sizeof(int) * n_spinlocks);
			assert(*spinlocks);

			for (int i = 0; i < n_spinlocks; i++)
				*((*spinlocks) + i) = 0;
		}

    fprintf(stderr, "init %d mutex, %d spinlocks\n",
    		n_mutex, n_spinlocks);
}

void free_locks(pthread_mutex_t *mutexes, int nmutex, int *spinlocks) {

  if (mutexes && nmutex) {
		for(int i = 0; i < nmutex; i++)
				pthread_mutex_destroy(&mutexes[i]);
		free(mutexes);
  }

  if (spinlocks)
  	free(spinlocks);
}

SortedList_t * alloc_lists(int n_lists) {
		assert(n_lists > 0);
		SortedList_t * lists = malloc(sizeof(SortedList_t) * n_lists);
		assert(lists);

    for (int i = 0; i < n_lists; i++){
        lists[i].prev = (SortedListElement_t *)&lists[i];
        lists[i].next = (SortedListElement_t *)&lists[i];
        lists[i].key = 0;
    }

// printf() with std redirect will result in a non-ASCII text file which Python failed to parse. Why? 
#if 1 
    fprintf(stderr, "init %d lists. sizeof(SortedList_t) = %lu "
    		"padding seems: %s\n",
    		n_lists, sizeof(SortedList_t),
				sizeof(SortedList_t) > sizeof(SortedListElement_t) ? "ON" : "OFF");
#endif

    //printf("%d\n", n_lists);
	//printf("hello world");
    return lists;
}

struct prog_config parse_config(int argc, char **argv) {
		struct prog_config config = {
				.iterations = -1,
				.numThreads = 1,
				.mutexFlag = 0,
				.numParts = -1
		};
    int opt = 0;

	   static struct option options [] = {
        {"iterations", required_argument, 0, 'i'},
        {"threads", required_argument, 0, 't'},
        {"sync", no_argument, 0, 's'},
        {"parts", required_argument, 0, 'p'},
        {0, 0, 0, 0}
    };

    while((opt=getopt_long(argc, argv, "i:t:sp:", options, NULL)) != -1){

        switch(opt){
            case 'i':
							{
								int len = strlen(optarg);
								int factor = 1;
								if (optarg[len-1] =='m' || optarg[len-1] == 'M') {
									factor = 1000 * 1000;
									optarg[len-1] ='\0';
								} else if (optarg[len-1] =='k' || optarg[len-1] == 'K') {
									factor = 1000;
									optarg[len-1] ='\0';
								}
                config.iterations = (int)atoi(optarg) * factor;
							}
                break;
            case 't':
            	config.numThreads = (int)atoi(optarg);
                break;
            case 's':
							config.mutexFlag = 1;
								break;
            case 'p':
                config.numParts = (int)atoi(optarg);
                break;
            default:
                usage();
                exit(1);
                break;
        }
    }

    if (config.iterations == -1) {
    	usage();
    	exit(1);
    }

    if (config.numParts == -1)  /* #parts unspecified, default = 1. biglock */
    	config.numParts = config.numThreads;

#if defined(USE_MULTILISTS) && defined(USE_LB)
#error "only one of USE_MULTILISTS and USE_LB can be compiled"
#endif

#ifdef USE_PREALLOC
    fprintf(stderr, "USE_PREALLOC=yes\n");
#else
    fprintf(stderr, "USE_PREALLOC=no\n");
#endif

#ifdef USE_MULTILISTS
    fprintf(stderr, "USE_MULTILISTS=yes\n");

    // load balancing not enabled, while #parts != #threads
		if (config.numParts != config.numThreads) {
			fprintf(stderr, "no load balancing (USE_LB undefined). # threads must equal # parts\n");
			exit(1);
		}

    config.numParts = config.numThreads;
#else
    fprintf(stderr, "USE_MULTILISTS=no\n");
#endif

#ifdef USE_LB
		fprintf(stderr, "USE_LB=yes\n");
#else
		fprintf(stderr, "USE_LB=no\n");
#endif

#ifdef USE_PADDING
    fprintf(stderr, "USE_PADDING=yes\n");
#else
    fprintf(stderr, "USE_PADDING=no\n");
#endif

    return config;
}
