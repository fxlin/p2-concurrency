#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <time.h>
#include <string.h>
#include <pthread.h>
#include <signal.h>
#include <errno.h>

#include "SortedList.h"

int iterations = 1;
int numThreads = 1;

int mutexFlag = 0;
int spinLockFlag = 0;

pthread_mutex_t mutex;

SortedList_t* list;
SortedListElement_t* elements;

int spinLock = 0;
int numElements = 0; // = numThreads * iterations

#define ONE_BILLION 1000000000L;

// XXX: improve this
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

void print_csv_line(char const* test, int threadNum, int iterations, int numList, int numOperation, long long runTime, long long avgTime){
//    fprintf(stdout, "%s,%d,%d,%d,%d,%lld,%lld\n", test, threadNum, iterations, numList, numOperation, runTime, avgTime);
	fprintf(stdout, "test=%s threadNum=%d iterations=%d numList=%d numOperation=%d runTime(ns)=%lld avgTime(ns)=%lld\n",
			test, threadNum, iterations, numList, numOperation, runTime, avgTime);
}

char const * getTestName() {
    if(mutexFlag)
    	return "list-m";
    if(spinLockFlag)
    	return "list-s";
    return "list-none";
}


//signal handler for SIGSEGV
void signal_handler(int sigNum){
    if(sigNum == SIGSEGV){
        print_errors("segfault");
    }
}

//https://stackoverflow.com/questions/19724346/generate-random-characters-in-c generating random characters in C
char* getRandomKey(){
    char* random_key = (char*) malloc(sizeof(char)*2);
    random_key[0] = (char) rand()%26 + 'a';
    random_key[1] = '\0';
    return random_key;
}

void initializeEmptyList(){
    list = (SortedList_t*) malloc(sizeof(SortedList_t));
    list->prev = list;
    list->next = list;
    list->key = NULL;
}

//initializes elements with random key
void initializeElements(int numElements){
    int i;
    for(i = 0; i < numElements; i++){
        elements[i].key = getRandomKey();
    }
}

void* thread_func(void* thread_id) {
    int i;
    int num = *((int*)thread_id);

    // insert ... each thread inserts a subset of elements.
    for(i = num; i < numElements; i += numThreads){
        if(mutexFlag){
            pthread_mutex_lock(&mutex);
            SortedList_insert(list, &elements[i]);
            pthread_mutex_unlock(&mutex);
        }
        else if(spinLockFlag){
            while(__sync_lock_test_and_set(&spinLock, 1));
            SortedList_insert(list, &elements[i]);
            __sync_lock_release(&spinLock);
        }
        else if(!spinLockFlag && !mutexFlag){
            SortedList_insert(list, &elements[i]);
        }
    }

    // get length. once
    int length = 0;
    if(mutexFlag){
        pthread_mutex_lock(&mutex);
        length = SortedList_length(list);
        if (length < 0){
            print_errors("length");
        }
        pthread_mutex_unlock(&mutex);
    }
    else if(spinLockFlag){
        while(__sync_lock_test_and_set(&spinLock, 1));
        length = SortedList_length(list);
        if (length < 0){
            print_errors("length");
        }
        __sync_lock_release(&spinLock);
    }
    else if(!spinLockFlag && !mutexFlag){
        length = SortedList_length(list);
        if (length < 0){
            print_errors("length");
        }
    }

    // lookup & del ... each thread looks up a subset of items.
    SortedListElement_t* insertedElement;
    num = *((int*)thread_id);
    for(i = num; i < numElements; i += numThreads) {
        if(mutexFlag){
            pthread_mutex_lock(&mutex);
            insertedElement = SortedList_lookup(list, elements[i].key);
            if(insertedElement == NULL){
                print_errors("lookup");
            }
            int res = SortedList_delete(insertedElement);
            if(res == 1){
                print_errors("delete");
            }
            pthread_mutex_unlock(&mutex);
        }
        else if(spinLockFlag){
            while(__sync_lock_test_and_set(&spinLock, 1));
            insertedElement = SortedList_lookup(list, elements[i].key);
            if(insertedElement == NULL){
                print_errors("lookup");
            }
            int res = SortedList_delete(insertedElement);
            if(res == 1){
                print_errors("delete");
            }
            __sync_lock_release(&spinLock);
        }
        else if(!mutexFlag && !spinLockFlag){
            insertedElement = SortedList_lookup(list, elements[i].key);
            if(insertedElement == NULL){
                print_errors("lookup");
            }
            int res = SortedList_delete(insertedElement);
            if(res == 1){
                print_errors("delete");
            }
        }
    }
    return NULL;
}

int main(int argc, char** argv){
    int opt = 0;
    char input;
    static struct option options [] = {
        {"iterations", 1, 0, 'i'},
        {"threads", 1, 0, 't'},
        {"yield", 1, 0, 'y'},
        {"sync", 1, 0, 's'},
        {0, 0, 0, 0}
    };
    while((opt=getopt_long(argc, argv, "i:t:y:s", options, NULL)) != -1){
        switch(opt){
            case 'i':
                iterations = (int)atoi(optarg);
                break;
            case 't':
                numThreads = (int)atoi(optarg);
                break;
            case 's':
                input = *optarg;
                switch(input){
                    case 'm':
                        if(pthread_mutex_init(&mutex, NULL) < 0){
                            print_errors("mutex");
                        }
                        mutexFlag = 1;
                        break;
                    case 's':
                        spinLockFlag = 1;
                        break;
                    default:
                        fprintf(stderr, "Incorrect argument\n");
                        exit(1);
                        break;
                }
                break;
            default:
                fprintf(stderr, "Incorrect argument\n");
                exit(1);
                break;
        }
        
    }

    signal(SIGSEGV, signal_handler);

    // prep elements for insert ...
    initializeEmptyList();
    numElements = numThreads * iterations;
    elements = (SortedListElement_t*) malloc(sizeof(SortedListElement_t)*numElements);
    if (!elements) {
    	printf("failed to allocate mem");
    	exit(2);
    }

    srand((unsigned int) time(NULL));
    initializeElements(numElements);


    int i;
    pthread_t threads[numThreads];
    int thread_id[numThreads];

    struct timespec start_time, end_time;
    if(clock_gettime(CLOCK_MONOTONIC, &start_time) < 0){
        print_errors("clock_gettime");
    }

    // launch threads
    for(i = 0; i < numThreads; i++){
        thread_id[i] = i;
        int rc = pthread_create(&threads[i], NULL, thread_func, &thread_id[i]);
        if(rc < 0){
            print_errors("thread_create");
        }
    }

    for(i = 0; i < numThreads; i++){
        int rc = pthread_join(threads[i], NULL);
        if(rc < 0){
            print_errors("thread_join");
        }
    }
    
    if(clock_gettime(CLOCK_MONOTONIC, &end_time) < 0){
        print_errors("clock_gettime");
    }
    
    long long diff =  (end_time.tv_sec - start_time.tv_sec) * ONE_BILLION;
    diff += end_time.tv_nsec;
    diff -= start_time.tv_nsec; //get run time

    // sanity check
    if(SortedList_length(list) != 0){
        print_errors("size");
    }

    /* output measurement */

    int numOpts = 3 * iterations * numThreads; // insert, lookup, deletion

    print_csv_line(getTestName(), numThreads, iterations, 1, numOpts, diff, diff/numOpts);

    for(i = 0; i < numElements; i++){
        free((void*)elements[i].key);
    }
    free(elements);
    free(list);

    if(mutexFlag)
        pthread_mutex_destroy(&mutex);

    exit(0);
}
