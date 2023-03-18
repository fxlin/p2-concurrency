#include <stdlib.h>
#include <pthread.h>
#include <stdbool.h>
#include "spinlock.h"

void lock_spinlock(spinlock* lock){
    // If is_locked is true initially, we will set is_locked to true and return true, so we continue looping.
    // If is_locked is false intially, we will set is_locked to true and return false, so we break out of the while loop.
    while(__atomic_test_and_set(lock->is_locked, __ATOMIC_SEQ_CST));
}

void unlock_spinlock(spinlock* lock){
    // Unlock by setting is_locked to false, so another thread can set it to true.
    __atomic_store_n(&(lock->is_locked), false, __ATOMIC_SEQ_CST);
}