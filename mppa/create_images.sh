#!/bin/bash

LOCAL_DIR=`pwd`
MPPA_TOOLCHAIN_DIR=/usr/local/k1tools

rm -fr bin
mkdir bin

cp $LOCAL_DIR/slave/bin/tsp_* $LOCAL_DIR/master/bin/tsp_* $LOCAL_DIR/bin/

cd  $LOCAL_DIR/bin/

createImage.rb --toolchain $MPPA_TOOLCHAIN_DIR/bin --clusters tsp_seq_mppa_slave --boot=tsp_seq_mppa_master -o tsp_seq_mppa_image
createImage.rb --toolchain $MPPA_TOOLCHAIN_DIR/bin --clusters tsp_lock_mppa_slave --boot=tsp_lock_mppa_master -o tsp_lock_mppa_image
#createImage.rb --toolchain $MPPA_TOOLCHAIN_DIR/bin --clusters tsp_cas_mppa_slave --boot=tsp_cas_mppa_master -o tsp_cas_mppa_image
