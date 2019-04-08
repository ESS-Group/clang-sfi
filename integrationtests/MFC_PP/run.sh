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

$EXECUTABLE $SOURCE_DIR/source.cpp --config=$SOURCE_DIR/config.json --patchdir=$BUILD_DIR/injections -p ../../build > $OUTPUT_FILE

AWK_OUTPUT="$(awk 'BEGIN {cnt=0}; {if(/Should not be removed/) {cnt+=1;}}; END {print cnt;}' injections/*)"
AWK_OUTPUT_TRIMMED="$(echo -e "${AWK_OUTPUT}" | sed -e 's/^[[:space:]]*//' -e 's/[[:space:]]*$//')"
echo $AWK_OUTPUT_TRIMMED | diff $REFERENCE_FILE - >> $OUTPUT_FILE || (echo "Test failed, reference output not given" && exit 1)
