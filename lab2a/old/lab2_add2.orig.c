//
//  lab2_add.c
//  cs111labs
//
//  Created by Luca Matsumoto on 4/28/18.
//  Copyright © 2018 Luca Matsumoto. All rights reserved.
//

#include <stdio.h>
#include <getopt.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>

#define ONE_BILLION 1000000000L;

long long counter = 0;
int iterations = 1;
int numThreads = 1;
int opt_yield;
char* test = "add-none";
int mutexFlag = 0;
int spinLockFlag = 0;
int c_and_sFlag = 0;
pthread_mutex_t mutex;
int spinLock = 0;

void getTestName(){
    if(opt_yield && !mutexFlag && !spinLockFlag && !c_and_sFlag){
        test = "add-yield-none";
    }
    else if(opt_yield && mutexFlag){
        test = "add-yield-m";
    }
    else if(opt_yield && spinLockFlag){
        test = "add-yield-s";
    }
    else if(opt_yield && c_and_sFlag){
        test = "add-yield-c";
    }
    else if(!opt_yield && mutexFlag){
        test = "add-m";
    }
    else if(!opt_yield && spinLockFlag){
        test = "add-s";
    }
    else if(!opt_yield && c_and_sFlag){
        test = "add-c";
    }
}

void cleanUpLocks(){
    if(mutexFlag){
        pthread_mutex_destroy(&mutex);
    }
}

void print_errors(char* error){
    if(strcmp(error, "clock_gettime") == 0){
        fprintf(stderr, "Error initializing clock time.\n");
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
        fprintf(stderr, "Error with pthread_join.\n");
        exit(2);
    }
}

void add(long long *pointer, long long value) {
    long long sum = *pointer + value;
    if (opt_yield)
        sched_yield();
    *pointer = sum;
}

void add_iterate(int val){
    int i;
    for(i = 0; i < iterations; i++){
        if(mutexFlag){
            pthread_mutex_lock(&mutex);
            add(&counter, val);
            pthread_mutex_unlock(&mutex);
        }
        else if(spinLockFlag){
            while(__sync_lock_test_and_set(&spinLock, 1));
            add(&counter, val);
            __sync_lock_release(&spinLock);
        }
        else if(c_and_sFlag){
            long long oldVal, newVal;
            do{
                oldVal = counter;
                newVal = oldVal + val;
                if(opt_yield){
                    sched_yield();
                }
            } while(__sync_val_compare_and_swap(&counter, oldVal, newVal) != oldVal);
        }
        else {
            add(&counter, val);
        }
    }
}//http://gcc.gnu.org/onlinedocs/gcc-4.4.3/gcc/Atomic-Builtins.html help with spin locks and compare and swap

void add_and_subtract(){
    add_iterate(1);
    add_iterate(-1);
}

void print_csv_line(char* test, int threadNum, int iterations, int numOperation, long long runTime, long long avgTime, long long count){
    fprintf(stdout, "%s,%d,%d,%d,%lld,%lld,%lld\n", test, threadNum, iterations, numOperation, runTime, avgTime, count);
}

int main(int argc, char** argv){
    int opt = 0;
    char input;
    static struct option options [] = {
        {"iterations", 1, 0, 'i'},
        {"threads", 1, 0, 't'},
        {"yield", 0, 0, 'y'},
        {"sync", 1, 0, 's'},
        {0, 0, 0, 0}
    };
    while((opt=getopt_long(argc, argv, "itys", options, NULL)) != -1){
        switch(opt){
            case 'i':
                iterations = (int)atoi(optarg);
                break;
            case 't':
                numThreads = (int)atoi(optarg);
                break;
            case 'y':
                opt_yield = 1;
                break;
            case 's':
                if(strlen(optarg) != 1){
                    fprintf(stderr, "Invalid option. Sync type must be one letter\n");
                    exit(1);
                }
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
                    case 'c':
                        c_and_sFlag = 1;
                        break;
                    default:
                        fprintf(stderr, "Incorrect argument: correct usage is ./lab2_add --iterations iterations --threads numberOfThreads [--yield] [--sync option] \n");
                        exit(1);
                        break;
                }
                break;
            default:
                fprintf(stderr, "Incorrect argument: correct usage is ./lab2_add --iterations iterations --threads numberOfThreads [--yield] [--sync option] \n");
                exit(1);
                break;
        }
        
    } //https://computing.llnl.gov/tutorials/pthreads/ for pthread help
    struct timespec start_time, end_time;
    
    if(clock_gettime(CLOCK_MONOTONIC, &start_time) < 0){
        print_errors("clock_gettime");
    }
    int i;
    pthread_t threads[numThreads];
    for(i = 0; i < numThreads; i++){
        int rc = pthread_create(&threads[i], NULL, (void*)&add_and_subtract, &counter);
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
    diff -= start_time.tv_nsec;
    int numOpts = 2*iterations*numThreads;
    getTestName();
    print_csv_line(test, numThreads, iterations, numOpts, diff, diff/numOpts, counter);
    atexit(cleanUpLocks);
    exit(0);
}
