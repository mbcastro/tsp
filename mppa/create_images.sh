#!/bin/bash

LOCAL_DIR=`pwd`
MPPA_TOOLCHAIN_DIR=/usr/local/k1tools

rm -fr bin
mkdir bin

cp $LOCAL_DIR/slave/bin/tsp_* $LOCAL_DIR/master/bin/tsp_* $LOCAL_DIR/bin/

cd  $LOCAL_DIR/bin/

k1-create-multibinary --clusters=tsp_lock_mppa_slave --boot=tsp_lock_mppa_master -T tsp_lock_mppa.mpk
