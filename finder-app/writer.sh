#!/bin/bash

writefile=$1
writestr=$2

if [ $# -ne 2 ]; then
    echo "Please check argument"
    exit 1
fi

mkdir -p $( dirname $writefile )
echo "$writestr" > $writefile

if [ $? -ne 0 ]; then
    exit 1
else
    exit 0
fi
