#!/bin/bash
# set -x 

# TODO: Switch between the following two to disable/enable VTUNE profiling
# VTUNE="vtune -collect hotspot -knob sampling-mode=hw"
VTUNE=

# TODO: Switch among the following three for # of iterations
#ITER=10k  # small
ITER=1M # med
#ITER=10M # large

run() {
  PROG=$1
  TRACEFILE=$2
  FACTOR=$3  # #of parts=factor x # of threads

      
  rm -rf $TRACEFILE
  touch $TRACEFILE
  
  echo $1 $2 $3
  
  # TODO: Set thread counts to test here
  # for tr in 1 2 4 6 8 10 12 16 20
  for tr in 1 2 4 6 8
  #for tr in 1 2
  do 
    $VTUNE $PROG --iterations=$ITER  --threads=$tr --parts=`expr $tr \* $FACTOR` >> $TRACEFILE 2>&1   
  done
  
  cat $TRACEFILE | grep "test="
}

#####################
# biglock
run "./list" "trace-.txt" 1

#####################
# malloc
#run "./lab2_list-malloc" "trace-malloc.txt" 1
run "./list-p" "trace-p.txt" 1

#####################
# no malloc, no stealing
#run "./lab2_list-steal" "trace-nomalloc-nosteal.txt" 1
run "./list-pm" "trace-pm.txt" 1

#####################
# no malloc, stealing
#run "./lab2_list-steal" "trace-nomalloc-steal.txt" 4
run "./list-pml" "trace-pml.txt" 4

#####################
# no malloc, stealing, padding
# run "./lab2_list-steal-padding" "trace-nomalloc-steal-padding.txt" 4
run "./list-pmla" "trace-pmla.txt" 4

#####################
# best
#run "./lab2_list-steal2-naivepadding" "lab2_list-steal2-naivepadding.txt" 4
