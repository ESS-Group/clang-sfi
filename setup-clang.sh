#!/bin/bash
set -ue

TOP_LEVEL_DIRECTORY=$(dirname "$(realpath "$0")")
BUILD_TYPE=Debug # or Release
if [[ $(uname -s) == CYGWIN* ]] || [[ $(uname -s) == MINGW* ]]; then
    WINDOWS=true
fi

cd "$TOP_LEVEL_DIRECTORY"
echo "Switched into $(pwd)"
cd llvm/tools
if [ ! -d "clang/.git" ]; then
    echo "Checking out clang into llvm/tools"
    git clone https://git.llvm.org/git/clang.git/
else
    echo "Fetching clang into llvm/tools"
    (cd clang && git fetch)
fi
(cd clang && git reset --hard aef981ef412f9b527292b2e911453d56d327e883)

if [ ! -z ${WINDOWS+x} ]; then
    cd "$TOP_LEVEL_DIRECTORY"
    if [ ! -d "zlib/.git" ]; then
        echo "Checking out zlib"
        git clone https://github.com/madler/zlib.git
    else
    	echo "Fetching zlib into zlib"
    	(cd zlib && git fetch)
    fi
    (cd zlib && git reset --hard cacf7f1d4e3d44d871b605da3b647f07d718623f)
fi

cd "$TOP_LEVEL_DIRECTORY"
if [ ! -d "src/libs/dtl/.git" ]; then
    echo "Checking out dtl"
    git clone https://github.com/cubicdaiya/dtl.git src/libs/dtl
else
    echo "Fetching dtl into src/libs/dtl"
    (cd src/libs/dtl && git fetch)
fi
(cd src/libs/dtl && git reset --hard 9cf6da72798e714307f63f416990dfc753fc94df)



cd "$TOP_LEVEL_DIRECTORY"
echo "Switched into $(pwd)"
if [ -d "llvm-build" ]; then
    DELETE="n"
    read -p "llvm-build already exists. Delete? [$DELETE]: " userInput
    if [[ -n "$userInput" ]]
    then
        DELETE=$userInput
    fi
    if [[ $DELETE =~ ^(yes|y|Y) ]]; then
        rm -Rf llvm-build
    fi
fi
mkdir -p llvm-build
cd llvm-build
echo "Created llvm-build"
if [ ! -z ${WINDOWS+x} ]; then
    cmake -G "Visual Studio 15 2017" -A x64 -Thost=x64 -DCMAKE_BUILD_TYPE=$BUILD_TYPE -DZLIB_INCLUDE_DIR=$(realpath ../zlib) -DZLIB_LIBRARY=$(realpath ../zlib-build/$BUILD_TYPE/zlibstatic.lib) ../llvm
else
    cmake -G Ninja ../llvm -DCMAKE_BUILD_TYPE=$BUILD_TYPE
fi
echo "Ran CMake"

COMPILE="n"
read -p "Should I start the compile? [$COMPILE]: " userInput
if [[ -n "$userInput" ]]
then
    COMPILE=$userInput
fi
if [[ $COMPILE =~ ^(yes|y|Y) ]]; then
    echo "Start compile"
    if [ ! -z ${WINDOWS+x} ]; then
        cmake --build . --target ALL_BUILD --config $BUILD_TYPE
    else
        ninja
    fi
fi

