#!/bin/bash
# set -x 

# sample commands for building, testing, and benchmarking hashtable
# tweak it to suit your need!

# compile
# gcc -o hashtable hashtable.c common.c measure.c SortedList.c -I /usr/local/include/glib-2.0 -lglib-2.0 -lpthread
gcc -o hashtable-biglock hashtable-biglock.c ../exp2/common.c ../exp2/measure.c -I../exp2/ `pkg-config --cflags glib-2.0` `pkg-config --libs glib-2.0` -lpthread


# TODO: Switch between the following two to disable/enable VTUNE profiling
# VTUNE="vtune -collect hotspot -knob sampling-mode=hw"
VTUNE="vtune -collect threading -knob sampling-and-waits=hw"
# VTUNE="vtune -collect uarch-exploration"

# TODO: Switch among the following three for # of iterations
#ITER=10k  # small
ITER=1M # med
# ITER=10M # large

run() {
  PROG=$1
  TRACEFILE=$2
  NUMHASHTABLE=$3
      
  rm -rf $TRACEFILE
  touch $TRACEFILE
  
  echo $1 $2
  
  # TODO: Set thread counts to test here
  for tr in 1 2 4 6 8 10 12 16 20
  # for tr in 1 2 4 6 8
  # for tr in 1 2
  do 
    $VTUNE $PROG --iterations=$ITER  --threads=$tr --parts=$tr $NUMHASHTABLE >> $TRACEFILE 2>&1   
  done
  
  cat $TRACEFILE | grep "test="
  echo "done"
}

# #####################
run "./hashtable" "hashtable-mono.txt" 1
run "./hashtable" "hashtable-100.txt" 100
run "./hashtable" "hashtable-1000.txt" 1000
run "./hashtable" "hashtable-10000.txt" 10000