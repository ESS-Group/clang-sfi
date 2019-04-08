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

$EXECUTABLE $SOURCE_DIR/source.cpp $SOURCE_DIR/library.cpp --config=$SOURCE_DIR/config.json --patchdir=$BUILD_DIR/injections -p ../../build > $OUTPUT_FILE
{ tail -n +3 $BUILD_DIR/injections/MFC_0.patch && tail -n +3 $BUILD_DIR/injections/MFC_1.patch; } | diff $REFERENCE_FILE - >> $OUTPUT_FILE || (echo "Test failed, reference output not given" && exit 1)
