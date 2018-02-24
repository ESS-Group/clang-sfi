cmake_minimum_required(VERSION 3.8)
set(PROJECT_NAME "clang-sfi")
project(${PROJECT_NAME})

SET(CMAKE_EXPORT_COMPILE_COMMANDS true)
SET(CMAKE_C_COMPILER clang)
SET(CMAKE_CXX_COMPILER clang++)
# Clang is compiled with -fno-rtti by default, so we have to use it too. (Could be activated in Clangs CMakeFile if needed.) Otherwise the compiler might try to use typeinfo symbols of Clang, which are just not there.
# Clang is compiled with -fvisibility=hidden. Compilers may generate weak symbols for template instantiations, which it does for the used clang templates inside of main.cc. This leads to weak symbols in main.o, which are visible by default (-fvisibility=default). Directly linking them to the library, which is the goal, triggers the warning that those weak symbols cannot be overriden at runtime, what might be desireable in other use-cases. Using -fvisibility=hidden hides the weak symbols and thus does not trigger the warning. (Maybe the reasoning is wrong... information are a bit sparse on this topic.)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-rtti -fvisibility=hidden")
set(CMAKE_CXX_STANDARD 14)

#set(LLVM_DIR /usr/local/opt/llvm/lib/cmake/llvm)
set(LLVMPATH /home/daniel/ba/faultclassifier)
set(LLVM_DIR "${LLVMPATH}/llvm-build/lib/cmake/llvm")
find_package(LLVM REQUIRED CONFIG)

find_package(ZLIB REQUIRED)

message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")
#include_directories(/usr/local/opt/llvm/include ./)
#link_directories(/usr/local/opt/llvm/lib)
link_directories("${LLVMPATH}/llvm-build/lib")
#include_directories(/Users/ug/llvm/llvm/tools/clang/include)
include_directories("${LLVMPATH}/llvm-build/tools/clang/include")
include_directories("${LLVMPATH}/llvm/tools/clang/include")
include_directories(${LLVM_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})

include_directories(src)
set(SOURCE_FILES src/StmtHandler.cpp src/FaultInjector.cpp src/SFIASTConsumer.cpp src/SFIAction.cpp)
add_library(${PROJECT_NAME}_lib ${SOURCE_FILES})
#the following line creates the entry in compile-commands for test.cpp
add_library(${PROJECT_NAME}_testfile src/test.cpp)
add_executable(${PROJECT_NAME} src/main.cpp)
target_link_libraries(${PROJECT_NAME} PRIVATE ${PROJECT_NAME}_lib)

# Find the libraries that correspond to the LLVM components
# that we wish to use
llvm_map_components_to_libnames(llvm_libs Support core irreader MC ProfileData Option AArch64AsmParser)

# Link against LLVM libraries
target_link_libraries(${PROJECT_NAME} PRIVATE ${llvm_libs})

file(GLOB ALL_MY_LLVM_LIBS "${LLVMPATH}/llvm-build/lib/*.a")
message(STATUS "Using following libs: ${ALL_MY_LLVM_LIBS}")
target_link_libraries(clang-sfi PRIVATE ${ALL_MY_LLVM_LIBS})
target_link_libraries(${PROJECT_NAME} PRIVATE ${ZLIB_LIBRARIES} curses)
#target_link_libraries(${PROJECT_NAME} PRIVATE clangBasic clangFrontend clangTooling clangToolingASTDiff clangToolingRefactor clangCrossTU clangIndex clangAST clangLex clangSerialization clangSema clangAnalysis clangARCMigrate clangEdit clangDriver clangParse)

#enable_testing()
#include(CTest)
#add_subdirectory(googletest)
#add_subdirectory(unittests)
#add_subdirectory(integrationtests)
