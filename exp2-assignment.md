# Scalability: exercises

## 0. Reproduce the benchmarks

Repeat what has been described in our writeup. 

Deliverable: a one-page report in PDF, in which you: 

* show the scalability plots you generated; 
* discuss your observation and compare with our results
* provide any explanation / reasoning needed 

## 1. The unfinished scalability quest

How does the list program scale to more than 8 cores? Profile its execution with VTune. It may be worth trying VTune's "microarchitecture exploration". Can you make it scale to 20 cores? If so, show your code and profiling results; if not, reason about about possible bottlenecks. 

Deliverable: 

A two-page report in PDF describing your observation. 

## 2. A scalable hashtable 

### Summary

Take an existing hashtable implementation in `glib` and make it scalable. 

`glib` is a widely used C library that implements common data structures, such as linked lists, trees, and hashtables. 

* The glib hashtable API is described [here](https://developer.gnome.org/glib/stable/glib-Hash-Tables.html). 
* For example usage of the API, see the glib's test code [here](https://github.com/GNOME/glib/blob/mainline/glib/tests/hash.c). 

Goal: write a benchmark program that spawn multiple threads for inserting keys to a hashtable concurrently. 

At the end of the benchmark, validate the correctness by checking all the keys in the hashtable, e.g. no missing or surplus keys. 

### Idea - a hashtable with internal partitions

<img src="figures/bigtable.png" style="zoom: 25%;" />

The "big lock" approach ensures correctness but cannot scale. We can build our own hashtable (called "bigtable") by wrapping around the glib's hashtable. A bigtable consists of *N* hashtables internally, where *N* is a parameter much larger than the number of threads. Each hashtable has its own lock which must be grabbed by a worker thread before inserting keys in the hashtable. 

To insert a key K, a worker thread first computes a hash function H(K) to determine which of the N hashtable the key should go. Then the worker thread grabs the lock for the hashtable, does the insertion, and unlock. Since N>>numThreads, the chance lock contention is low and our bigtable should scale better than the "big lock" approach. 

Of course, there are many details to take care of. I hope the tutorial above can give some useful pointers. 

### A sample plan of the attack 

1. Single-threaded. write the benchmark program using the glib hashtable API. Make sure you understand how to operate the hashtable. 
2. Multi-threaded with a big lock. Transform the above version by adding pthread, mutex, etc. Design test to validate the correctness of the resultant hashtable. 
3. Add internal partitions. 
4. Test & profile. 