/*
 * common.h
 *
 *  Created on: Jun 25, 2020
 *      Author: xzl
 */

#ifndef LAB2B_COMMON_H_
#define LAB2B_COMMON_H_

#include <pthread.h>

//typedef unsigned __int128  my_key_t; // diffcult to printf
typedef unsigned long  my_key_t;

struct SortedListElement {
    struct SortedListElement *prev;
    struct SortedListElement *next;
    my_key_t key;
};

typedef struct SortedListElement SortedListElement_t;

#ifdef USE_PADDING
struct SortedListElement_padding {
  struct SortedListElement *prev;
  struct SortedListElement *next;
  my_key_t key;
  char padding[16 * 4096];
};

typedef struct SortedListElement_padding SortedList_t;
#else
typedef struct SortedListElement SortedList_t;
#endif

/**
 * SortedList_insert ... insert an element into a sorted list
 *
 *    The specified element will be inserted in to
 *    the specified list, which will be kept sorted
 *    in ascending order based on associated keys
 *
 * @param SortedList_t *list ... header for the list
 * @param SortedListElement_t *element ... element to be added to the list
 */
void SortedList_insert(SortedList_t *list, SortedListElement_t *element);


/**
 * SortedList_length ... count elements in a sorted list
 *    While enumeratign list, it checks all prev/next pointers
 *
 * @param SortedList_t *list ... header for the list
 *
 * @return int number of elements in list (excluding head)
 *       -1 if the list is corrupted
 */
int SortedList_length(SortedList_t *list);

my_key_t getRandomKey();

struct prog_config {
	int iterations;
	int numThreads;
	int mutexFlag;
//	int spinLockFlag;
	int numParts; // task granularity
};

typedef struct prog_config prog_config;

void getTestName(struct prog_config *config, char *buf, int buflen);

void print_csv_line(char* test, int threadNum, int iterations,
		int numList, int numOperation, long long runTime);

struct prog_config parse_config(int argc, char **argv);

void alloc_locks(pthread_mutex_t **mutexes, int n_mutex,
		int **spinlocks, int n_spinlocks);

void free_locks(pthread_mutex_t *mutexes, int nmutex, int *spinlocks);

SortedList_t * alloc_lists(int n_lists);

SortedListElement_t *alloc_elements(int numElements);

void init_keys(my_key_t *keys, int numElements);

#endif /* LAB2B_COMMON_H_ */
