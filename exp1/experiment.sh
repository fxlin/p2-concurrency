#!/usr/bin/bash
for i in {0..9}:
do
    echo "Trial $i "
    ./counter-nolock --iterations=100000 --threads=10
    ./counter --iterations=100000 --threads=10 --sync=m
    ./counter --iterations=100000 --threads=10 --sync=s
    ./counter --iterations=100000 --threads=10 --sync=c
    echo ""
done