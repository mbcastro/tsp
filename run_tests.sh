#!/bin/bash

for THREADS in {2..16}; do 
    for PARTITIONS in {4..24}; do 
	./x86/bin/tsp_lock $THREADS 8 123 $PARTITIONS idchire >> out.txt
    done 
done

for THREADS in {2..16}; do 
    for PARTITIONS in {4..24}; do 
	./x86/bin/tsp_lock $THREADS 16 321 $PARTITIONS idchire >> out.txt
    done 
done

for THREADS in {2..16}; do 
    for PARTITIONS in {4..24}; do 
	./x86/bin/tsp_lock $THREADS 20 213 $PARTITIONS idchire >> out.txt
    done 
done
