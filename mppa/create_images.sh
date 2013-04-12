#!/bin/bash

LOCAL_DIR=`pwd`
MPPA_TOOLCHAIN_DIR=/usr/local/k1tools

mkdir bin > /dev/null

createImage.rb --toolchain $MPPA_TOOLCHAIN_DIR/bin --clusters $LOCAL_DIR/slave/bin/tsp_seq_mppa_slave --boot=$LOCAL_DIR/master/bin/tsp_seq_mppa_master -o $LOCAL_DIR/bin/tsp_seq_mppa_image
createImage.rb --toolchain $MPPA_TOOLCHAIN_DIR/bin --clusters $LOCAL_DIR/slave/bin/tsp_lock_mppa_slave --boot=$LOCAL_DIR/master/bin/tsp_lock_mppa_master -o $LOCAL_DIR/bin/tsp_lock_mppa_image
createImage.rb --toolchain $MPPA_TOOLCHAIN_DIR/bin --clusters $LOCAL_DIR/slave/bin/tsp_cas_mppa_slave --boot=$LOCAL_DIR/master/bin/tsp_cas_mppa_master -o $LOCAL_DIR/bin/tsp_cas_mppa_image
