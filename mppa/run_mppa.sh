#!/bin/bash

# For the last parameter remember to add a leading 0 otherwise the
# k1-jtag-runner will ignore the last parameter
#
# Ex, to run with two clusters:
#
# ./run_mppa.sh bin/tsp_lock_mppa.mpk tsp_lock_mppa_master 16 10 122 02

k1-jtag-runner --multibinary=$1 --exec-multibin=IODDR0:$2 ${*:3}
