#!/bin/bash

REPETITIONS=1

for REP in {1..$REPETITIONS}; do
    for THREADS in {1..16}; do 
	../../tools/likwid-3.0/likwid-powermeter -c 1 ./x86/bin/tsp_lock $THREADS 16 122 1 1 idchire >> idchire.txt
    done
done

for REP in {1..$REPETITIONS}; do
    for PARTITIONS in {2..12}; do 
	../../tools/likwid-3.0/likwid-powermeter -c $PARTITIONS ./x86/bin/tsp_lock 16 18 122 $PARTITIONS 1 idchire >> idchire.txt
    done 
done

for REP in {1..$REPETITIONS}; do
    for PARTITIONS in {13..24}; do 
	../../tools/likwid-3.0/likwid-powermeter -c $PARTITIONS ./x86/bin/tsp_lock 16 20 122 $PARTITIONS 1 idchire >> idchire.txt
    done 
done
