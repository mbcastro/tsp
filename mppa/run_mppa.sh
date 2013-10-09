#!/bin/bash

#k1-jtag-runner --exec-file=IODDR0:$1 ${*:2} 
k1-jtag-runner --multibinary=$1 --exec-multibin=IODDR0:$2 ${*:3}
