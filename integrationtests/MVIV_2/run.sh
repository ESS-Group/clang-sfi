#!/bin/bash
set -ue
cd "$(dirname "$0")"

# $2 is CMAKE_CURRENT_SOURCE_DIR
# $3 is CMAKE_CURRENT_BINARY_DIR
OUTPUT_FILE=$3/output.txt
REFERENCE_FILE=$2/reference.txt

$1 $2/source.cpp --config=$2/config.json --dir=$3/injections -- > $OUTPUT_FILE
ls $3/injections/* | wc -l | diff $REFERENCE_FILE - >> $OUTPUT_FILE || (echo "Test failed, reference output not given" && exit 1)
