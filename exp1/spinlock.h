
#ifndef SPINLOCK_H
#define SPINLOCK_H

#include <stdbool.h>

struct spinlock{
    bool is_locked;
} typedef spinlock;

void lock_spinlock(spinlock* lock);

void unlock_spinlock(spinlock* lock);

#endif