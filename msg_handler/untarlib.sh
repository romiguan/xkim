#!/bin/bash

BASE_DIR=$1
FILE=$2
if [ ! -d $BASE_DIR/lib -o ! -d $BASE_DIR/src ]
then
    tar zxvf $BASE_DIR/$2
fi
