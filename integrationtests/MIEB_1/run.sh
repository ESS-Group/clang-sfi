#!/bin/bash
set -ue
cd "$(dirname "$0")"

source ../init.sh
# Sets:
# $SOURCE_DIR
# $BUILD_DIR
# $EXECUTABLE

OUTPUT_FILE=$BUILD_DIR/output.txt
REFERENCE_FILE=$SOURCE_DIR/reference.txt

$EXECUTABLE $SOURCE_DIR/source.cpp --config=$SOURCE_DIR/config.json --dir=$BUILD_DIR/injections -- > $OUTPUT_FILE
tail -n +3 $BUILD_DIR/injections/MIEB_0.patch | diff $REFERENCE_FILE - >> $OUTPUT_FILE || (echo "Test failed, reference output not given" && exit 1)
