#!/bin/bash
set -ue
cd "$(dirname "$0")"

source ../init.sh
# Sets:
# $SOURCE_DIR
# $BUILD_DIR
# $EXECUTABLE

OUTPUT_FILE=$BUILD_DIR/output.txt
OUTPUT_FILE2=$BUILD_DIR/output2.txt
REFERENCE_FILE=$SOURCE_DIR/reference.txt
REFERENCE_FILE2=$SOURCE_DIR/reference2.txt

# Check that injection is performed.
$EXECUTABLE $SOURCE_DIR/source.cpp --config=$SOURCE_DIR/config.json --patchdir=$BUILD_DIR/injections -- > $OUTPUT_FILE
tail -n +3 $BUILD_DIR/injections/MFC_0.patch | diff $REFERENCE_FILE - >> $OUTPUT_FILE || (echo "Test failed, reference output not given" && exit 1)

# Check that it is performed in header file.
WC_OUTPUT="$(cat $BUILD_DIR/injections/MFC_0.patch | grep "source.h" | wc -l)"
WC_OUTPUT_TRIMMED="$(echo -e "${WC_OUTPUT}" | sed -e 's/^[[:space:]]*//' -e 's/[[:space:]]*$//')"
echo $WC_OUTPUT_TRIMMED | diff $REFERENCE_FILE2 - >> $OUTPUT_FILE2 || (echo "Test failed, reference output not given" && exit 1)
