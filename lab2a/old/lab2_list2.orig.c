#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <time.h>
#include <string.h>
#include "SortedList.h"
#include <pthread.h>
#include <signal.h>
#include <errno.h>

int iterations = 1;
int numThreads = 1;
int mutexFlag = 0;
int spinLockFlag = 0;
pthread_mutex_t mutex;
SortedList_t* list;
SortedListElement_t* elements;
int spinLock = 0;
int numElements = 0;
char test[32] = "list-";
int opt_yield = 0;
char* yieldOpts = NULL;

#define ONE_BILLION 1000000000L;

void print_csv_line(char* test, int threadNum, int iterations, int numList, int numOperation, long long runTime, long long avgTime){
    fprintf(stdout, "%s,%d,%d,%d,%d,%lld,%lld\n", test, threadNum, iterations, numList, numOperation, runTime, avgTime);
}

void getTestName(){
    if(yieldOpts == NULL){
        yieldOpts = "none";
    }
    strcat(test, yieldOpts);
    if(mutexFlag){
        strcat(test, "-m");
    }
    else if(spinLockFlag){
        strcat(test, "-s");
    }
    else if(!mutexFlag && !spinLockFlag){
        strcat(test, "-none");
    }
}

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

void initializeEmptyList(){
    list = (SortedList_t*) malloc(sizeof(SortedList_t));
    list->prev = list;
    list->next = list;
    list->key = NULL;
}

void initializeElements(int numElements){
    int i;
    for(i = 0; i < numElements; i++){
        elements[i].key = getRandomKey();
    }
} //initializes elements with random key

void* sorted_list_action(void* thread_id){
    int i;
    int num = *((int*)thread_id);
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
    SortedListElement_t* insertedElement;
    num = *((int*)thread_id);
    for(i = num; i < numElements; i += numThreads){
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
                        if(pthread_mutex_init(&mutex, NULL) < 0){
                            print_errors("mutex");
                        }
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
            default:
                fprintf(stderr, "Incorrect argument: correct usage is ./lab2_list --iterations iterations --threads numberOfThreads --yield optString [--sync option] \n");
                exit(1);
                break;
        }
        
    }//https://computing.llnl.gov/tutorials/pthreads/ for pthread help
    signal(SIGSEGV, signal_handler);
    initializeEmptyList();
    numElements = numThreads*iterations;
    elements = (SortedListElement_t*) malloc(sizeof(SortedListElement_t)*numElements);
    srand((unsigned int) time(NULL)); //must use srand before rand
    initializeElements(numElements);

    struct timespec start_time, end_time;
    if(clock_gettime(CLOCK_MONOTONIC, &start_time) < 0){
        print_errors("clock_gettime");
    }
    
    int i;
    pthread_t threads[numThreads];
    int thread_id[numThreads];
    
    for(i = 0; i < numThreads; i++){
        thread_id[i] = i;
        int rc = pthread_create(&threads[i], NULL, sorted_list_action, &thread_id[i]);
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
    
    if(SortedList_length(list) != 0){
        print_errors("size");
    }
    
    long long diff =  (end_time.tv_sec - start_time.tv_sec) * ONE_BILLION;
    diff += end_time.tv_nsec;
    diff -= start_time.tv_nsec; //get run time

    int numOpts = 3*iterations*numThreads; //get number of operations

    getTestName();
    
    print_csv_line(test, numThreads, iterations, 1, numOpts, diff, diff/numOpts);
    for(i = 0; i < numElements; i++){
        free((void*)elements[i].key);
    }
    free(elements);
    free(list);
    if(mutexFlag){
        pthread_mutex_destroy(&mutex);
    }
    exit(0);
}



