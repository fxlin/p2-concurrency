#!/bin/bash
# set -x 

# sample commands for running 
# tweak it to suit your need!

# NOTE: Uncomment any to choose profiling type. Comment all to disable profiling. 
# VTUNE="vtune -collect hotspot -knob sampling-mode=hw"
# VTUNE="vtune -collect threading -knob sampling-and-waits=hw"
# VTUNE="vtune -collect uarch-exploration"

# NOTE: Switch among the following three to change # of iterations
ITER=10k  # small
#ITER=1M # med
# ITER=10M # large

run() {
  PROG=$1
  TRACEFILE=$2
  NUMHASHTABLE=$3
      
  rm -rf $TRACEFILE
  touch $TRACEFILE
  
  echo $1 $2
  
  # NOTE: Set thread counts to test 
  # for tr in 1 2 4 6 8 10 12 16 20
  # for tr in 1 2 4 6 8
  for tr in 1 2
  do 
    $VTUNE $PROG --iterations=$ITER  --threads=$tr --parts=$tr $NUMHASHTABLE >> $TRACEFILE 2>&1   
  done
  
  cat $TRACEFILE | grep "test="
  echo "All tests done"
}

# #####################
mkdir -p output/
run "./hashtable-biglock" "output/hashtable-mono.txt" 1
#run "./hashtable" "output/hashtable-100.txt" 100
#run "./hashtable" "output/hashtable-1000.txt" 1000
#run "./hashtable" "output/hashtable-10000.txt" 10000