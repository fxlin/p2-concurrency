// # of parts == # of threads. padding can be on/off.
// no big lock

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <time.h>
#include <assert.h>
#include <string.h>
#include "SortedList.h"
#include <pthread.h>
#include <signal.h>
#include <errno.h>

#ifdef USE_VTUNE
#include <ittnotify.h>
#endif

#include "measure.h"  // xzl

int iterations = 1;
int numThreads = 1;
int numList = 1;
int mutexFlag = 0;
int spinLockFlag = 0;
pthread_mutex_t* mutexes;
SortedList_t* lists;
SortedListElement_t* elements;
int* spinLocks;
int numElements = 0;
char test[32];
int opt_yield = 0;
char* yieldOpts = NULL;
long long runTime = 0;
int* nums;	// xzl: this is a lookup table. a bit silly design

#ifdef USE_VTUNE
	__itt_domain * itt_domain = NULL;
	__itt_string_handle * sh_sort = NULL; // the overall task name
	__itt_string_handle ** sh_parts = NULL; // per part task name
#endif

#define ONE_BILLION 1000000000L;

#if 0
char const * getTestName() {
    if(mutexFlag)
    	return "list-m";
    if(spinLockFlag)
    	return "list-s";

    return "list-none";
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

char* getRandomKey(){
    char* random_key = (char*) malloc(sizeof(char)*2);
    random_key[0] = (char) rand()%26 + 'a';
    random_key[1] = '\0';
    return random_key;
}//https://stackoverflow.com/questions/19724346/generate-random-characters-in-c generating random characters in C

int hashFunction(const char* keys){
    return keys[0]%numList;
}

void setupLocks(){
//    if(mutexFlag){
        mutexes = malloc(sizeof(pthread_mutex_t)*numList);
//    }
    if(spinLockFlag){
        spinLocks = malloc(sizeof(int)*numList);
    }
    int i;
    for(i = 0; i < numList; i++){
//        if(mutexFlag){
            if(pthread_mutex_init(&mutexes[i], NULL) < 0){
                print_errors("mutex");
            }
//        }
        if(spinLockFlag){
            spinLocks[i] = 0;
        }
        else {
            break;
        }
    }

    printf("init %d locks\n", numList);
}


void initializeSubLists(){
    lists = malloc(sizeof(SortedList_t)*numList);
    int i;
    for (i = 0; i < numList; i++){
        lists[i].prev = &lists[i];
        lists[i].next = &lists[i];
        lists[i].key = NULL;
    }
}

void initializeElements(int numElements){
    int i;
    for(i = 0; i < numElements; i++){
        elements[i].key = getRandomKey();
    }
} //initializes elements with random key

void initializeLists() {
    initializeSubLists();
    setupLocks();
    initializeElements(numElements);
//    splitElements();
}//method to initialize lists, keys, locks

void* sorted_list_action(void* thread_id){
//    struct timespec start_time, end_time;
    int i,j;
    int num = *((int*)thread_id);

    pthread_mutex_lock(&mutexes[0]);
    k2_measure("tr start");
    pthread_mutex_unlock(&mutexes[0]);

#ifdef USE_VTUNE
    __itt_task_begin(itt_domain, __itt_null, __itt_null, sh_parts[num]);
#endif

    //printf("i'm thread %d\n", num);

    /* insert */
    int per_tr = numElements / numThreads;
    for(i = per_tr * num; i < per_tr * (num + 1); i ++) {
        if(mutexFlag) {
						pthread_mutex_lock(&mutexes[num]);
						SortedList_insert(&lists[num], &elements[i]);
						pthread_mutex_unlock(&mutexes[num]);
				}
        else if(spinLockFlag){
            while(__sync_lock_test_and_set(&spinLocks[num], 1));
            SortedList_insert(&lists[num], &elements[i]);
            __sync_lock_release(&spinLocks[num]);
        }
        /* no lock */
        else if(!spinLockFlag && !mutexFlag){
        	SortedList_insert(&lists[num], &elements[i]);
        }
    }

    /* length */
#if 0 /* could be kept?? */
    int length = 0;
    if(mutexFlag){
        for(j = 0; j < numList; j++){
            if(clock_gettime(CLOCK_MONOTONIC, &start_time) < 0){
                print_errors("clock_gettime");
            }
            pthread_mutex_lock(&mutexes[j]);
            if(clock_gettime(CLOCK_MONOTONIC, &end_time) < 0){
                print_errors("clock_gettime");
            }
            runTime +=  (end_time.tv_sec - start_time.tv_sec) * ONE_BILLION;
            runTime += end_time.tv_nsec;
            runTime -= start_time.tv_nsec; //get run time
            int subLength = SortedList_length(&lists[j]);
            if (subLength < 0){
                print_errors("length");
            }
            length += subLength;
            pthread_mutex_unlock(&mutexes[j]);
        }
    }
    else if(spinLockFlag){
        for(j = 0; j < numList; j++){
            if(clock_gettime(CLOCK_MONOTONIC, &start_time) < 0){
                print_errors("clock_gettime");
            }
            while(__sync_lock_test_and_set(&spinLocks[j], 1));
            if(clock_gettime(CLOCK_MONOTONIC, &end_time) < 0){
                print_errors("clock_gettime");
            }
            runTime +=  (end_time.tv_sec - start_time.tv_sec) * ONE_BILLION;
            runTime += end_time.tv_nsec;
            runTime -= start_time.tv_nsec; //get run time
            int subLength = SortedList_length(&lists[j]);
            if (subLength < 0){
                print_errors("length");
            }
            length += subLength;
            __sync_lock_release(&spinLocks[j]);
        }
    }
    else if(!spinLockFlag && !mutexFlag){
        for(j = 0; j < numList; j++){
            int subLength = SortedList_length(&lists[j]);
            if (subLength < 0){
                print_errors("length");
            }
            length += subLength;
        }
    }
#endif

    /* lookup + deletion */
#if 0
    SortedListElement_t* insertedElement;
    num = *((int*)thread_id);
    for(i = num; i < numElements; i += numThreads){
        if(mutexFlag){
            if(clock_gettime(CLOCK_MONOTONIC, &start_time) < 0){
                print_errors("clock_gettime");
            }
            pthread_mutex_lock(&mutexes[nums[i]]);
            if(clock_gettime(CLOCK_MONOTONIC, &end_time) < 0){
                print_errors("clock_gettime");
            }
            runTime +=  (end_time.tv_sec - start_time.tv_sec) * ONE_BILLION;
            runTime += end_time.tv_nsec;
            runTime -= start_time.tv_nsec; //get run time
            insertedElement = SortedList_lookup(&lists[nums[i]], elements[i].key);
            if(insertedElement == NULL){
                print_errors("lookup");
            }
            int res = SortedList_delete(insertedElement);
            if(res == 1){
                print_errors("delete");
            }
            pthread_mutex_unlock(&mutexes[nums[i]]);
        }
        else if(spinLockFlag){
            if(clock_gettime(CLOCK_MONOTONIC, &start_time) < 0){
                print_errors("clock_gettime");
            }
            while(__sync_lock_test_and_set(&spinLocks[nums[i]], 1));
            if(clock_gettime(CLOCK_MONOTONIC, &end_time) < 0){
                print_errors("clock_gettime");
            }
            runTime +=  (end_time.tv_sec - start_time.tv_sec) * ONE_BILLION;
            runTime += end_time.tv_nsec;
            runTime -= start_time.tv_nsec; //get run time
            insertedElement = SortedList_lookup(&lists[nums[i]], elements[i].key);
            if(insertedElement == NULL){
                print_errors("lookup");
            }
            int res = SortedList_delete(insertedElement);
            if(res == 1){
                print_errors("delete");
            }
            __sync_lock_release(&spinLocks[nums[i]]);
        }
        else if(!mutexFlag && !spinLockFlag){
            insertedElement = SortedList_lookup(&lists[nums[i]], elements[i].key);
            if(insertedElement == NULL){
                print_errors("lookup");
            }
            int res = SortedList_delete(insertedElement);
            if(res == 1){
                print_errors("delete");
            }
        }
    }
#endif // if 0

    pthread_mutex_lock(&mutexes[0]);
    k2_measure("tr done");
    pthread_mutex_unlock(&mutexes[0]);

#ifdef USE_VTUNE
    __itt_task_end(itt_domain);
#endif
    return NULL;
}

int main(int argc, char** argv){
    int opt = 0;
    char input;
    static struct option options [] = {
        {"iterations", 1, 0, 'i'},
        {"threads", 1, 0, 't'},
        {"sync", 1, 0, 's'},
        {"lists", 1, 0, 'l'},
        {0, 0, 0, 0}
    };
    while((opt=getopt_long(argc, argv, "i:t:ysl:", options, NULL)) != -1){
        switch(opt){
            case 'i':
                iterations = (int)atoi(optarg);
                break;
            case 't':
                numThreads = (int)atoi(optarg);
                break;
            case 'y':
                yieldOpts = (char*) malloc(sizeof(char)*6);
                yieldOpts = strdup(optarg);
                size_t i;
                for(i = 0; i < strlen(optarg); i++){
                    switch(optarg[i]){
                        case 'i':
                            opt_yield |= INSERT_YIELD;
                            break;
                        case 'd':
                            opt_yield |= DELETE_YIELD;
                            break;
                        case 'l':
                            opt_yield |= LOOKUP_YIELD;
                            break;
                        default:
                            fprintf(stderr, "Invalid argument for yield option. \n");
                            exit(1);
                        }
                }
                break;
            case 's':
                input = *optarg;
                switch(input){
                    case 'm':
                        mutexFlag = 1;
                        break;
                    case 's':
                        spinLockFlag = 1;
                        break;
                    default:
                        fprintf(stderr, "Incorrect argument: correct usage is ./lab2_list --iterations iterations --threads numberOfThreads --yield optString [--sync option] \n");
                        exit(1);
                        break;
                }
                break;
            case 'l':
                numList = (int)atoi(optarg);
                break;
            default:
                fprintf(stderr, "Incorrect argument: correct usage is ./lab2_list --iterations iterations --threads numberOfThreads --yield optString [--sync option] \n");
                exit(1);
                break;
        }
        
    }//https://computing.llnl.gov/tutorials/pthreads/ for pthread help
    signal(SIGSEGV, signal_handler);

    k2_measure("init");

    numElements = numThreads*iterations;
    elements = (SortedListElement_t*) malloc(sizeof(SortedListElement_t)*numElements);
    srand((unsigned int) time(NULL)); //must use srand before rand
    initializeLists();
    
#ifdef USE_VTUNE
    itt_domain = __itt_domain_create("my domain");
		__itt_thread_set_name("my main");

		// pre create here, instead of doing it inside tasks
		sh_parts = malloc(sizeof(__itt_string_handle *) * numThreads);
		assert(sh_parts);
		char itt_task_name[32];
		for (int i = 0; i < numThreads; i++) {
			snprintf(itt_task_name, 32, "part-%d", i);
			sh_parts[i] = __itt_string_handle_create(itt_task_name);
		}
#endif

    k2_measure("init done");

    int i;
    pthread_t threads[numThreads];
    int thread_id[numThreads];
    struct timespec start, end;
    if (clock_gettime(CLOCK_MONOTONIC, &start) < 0){
        print_errors("clock_gettime");
    }
    
#ifdef USE_VTUNE
//    sh_sort = __itt_string_handle_create("list");
//    __itt_string_handle * shMyTask = sh_sort;
//		__itt_task_begin(itt_domain, __itt_null, __itt_null, shMyTask);
    __itt_task_begin(itt_domain, __itt_null, __itt_null,
    					__itt_string_handle_create("list"));
#endif

    for(i = 0; i < numThreads; i++){
        thread_id[i] = i;
        int rc = pthread_create(&threads[i], NULL, sorted_list_action, &thread_id[i]);
        if(rc < 0){
            print_errors("thread_create");
        }
    }

    k2_measure("tr launched");

    for(i = 0; i < numThreads; i++){
        int rc = pthread_join(threads[i], NULL);
        if(rc < 0){
            print_errors("thread_join");
        }
    }
    
#ifdef USE_VTUNE
	__itt_task_end(itt_domain);
	free(sh_parts);
#endif

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
			for(i = 0; i < numList; i++){
					int ll = SortedList_length(&lists[i]);
					fprintf(stderr, "list %d: %d items; ", i, ll);
					total += ll;
			}
			fprintf(stderr, "\ntotal %ld items\n", total);
    }


    k2_measure_flush();

    int numOpts = 3 * iterations * numThreads; //get number of operations
    int numLockOpts = (2*iterations + 1) * numThreads;

    getTestName();
    
    print_csv_line(test, numThreads, iterations, numList, numOpts, diff, diff/numOpts, runTime/numLockOpts);
    
    for(i = 0; i < numElements; i++){
        free((void*)elements[i].key);
    }
    free(elements);
    free(lists);

    if(mutexFlag){
        free(mutexes);
        for(i = 0; i < numList; i++){
            pthread_mutex_destroy(&mutexes[i]);
        }
    }
    if(spinLockFlag){
        free(spinLocks);
    }

    free(nums);
    exit(0);
}



