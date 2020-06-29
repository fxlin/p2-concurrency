#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sched.h>
#include "SortedList.h"

void SortedList_insert(SortedList_t *list, SortedListElement_t *element) {
    if (!list || !element) {
        return;
    }

    SortedListElement_t *curr = list->next;

//    if (opt_yield & INSERT_YIELD){
//        sched_yield();
//    }

#if 0
    while(curr != list){
        if (strcmp(element->key, curr->key) <= 0){
            break;
        }
        curr = curr->next;
    }
#endif

    element->prev = curr->prev;
    element->next = curr;
    curr->prev->next = element;
    curr->prev = element;
}


int SortedList_delete(SortedListElement_t *element){
    if ((element->next->prev != element) || (element->prev->next != element)) {
          return 1;
    }
//    if (opt_yield & DELETE_YIELD) {
//        sched_yield();
//    }
    element->next->prev = element->prev;
    element->prev->next = element->next;
    return 0;
}

SortedListElement_t *SortedList_lookup(SortedList_t *list, my_key_t const key){
    SortedListElement_t *curr = list->next;
//    if (opt_yield & LOOKUP_YIELD) {
//        sched_yield();
//    }
    while(curr != (SortedListElement_t *)list) {
//        if (strcmp(curr->key, key) == 0){
    		if (curr->key == key)
            return curr;
        curr = curr->next;
    }
    return NULL;
}

int SortedList_length(SortedList_t *list){
    SortedListElement_t *curr = list->next;
    int length = 0;
//    if (opt_yield & LOOKUP_YIELD) {
//        sched_yield();
//    }
    while(curr != (SortedListElement_t *)list) {
        if ((curr->next->prev != curr) || (curr->prev->next != curr)){
            return -1;
        }
        length++;
        curr = curr->next;
    }
    return length;
}

// aux
my_key_t getRandomKey() {
	my_key_t ret = 0;
	// per byte
	for (unsigned int i = 0; i < sizeof(ret); i++) {
		ret  |= rand() & 0xff;
		ret <<= 8 ;
	}
	return ret;
}
