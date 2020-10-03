#!/bin/bash
# set -x 

# VTUNE="vtune -collect hotspot -knob sampling-mode=hw"
VTUNE=

#ITER=10k  # small
ITER=1M # med
#ITER=10M # large

run() {
  PROG=$1
  TRACEFILE=$2
  FACTOR=$3

      
  rm -rf $TRACEFILE
  touch $TRACEFILE
  
  echo $1 $2 $3
  
  for tr in 1 2 4 8 16 20
  #for tr in 1 2 
  do 
    $VTUNE $PROG --iterations=$ITER  --threads=$tr --parts=`expr $tr \* $FACTOR` >> $TRACEFILE 2>&1   
  done
  
  cat $TRACEFILE | grep "test="
}

#####################
# biglock
#run "./lab2_list-biglock" "trace-biglock.txt" 1
run "./list" "trace.txt" 1

#####################
# malloc
#run "./lab2_list-malloc" "trace-malloc.txt" 1
run "./list-p" "trace.txt" 1

#####################
# no malloc, no stealing
#run "./lab2_list-steal" "trace-nomalloc-nosteal.txt" 1

#####################
# no malloc, stealing
#run "./lab2_list-steal" "trace-nomalloc-steal.txt" 4

#####################
# no malloc, stealing, padding
# run "./lab2_list-steal-padding" "trace-nomalloc-steal-padding.txt" 4

#####################
# best
#run "./lab2_list-steal2-naivepadding" "lab2_list-steal2-naivepadding.txt" 4
