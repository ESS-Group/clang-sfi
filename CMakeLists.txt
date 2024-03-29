cmake_minimum_required(VERSION 3.7)
set(PROJECT_NAME "clang-sfi")
project(${PROJECT_NAME})

###
# Some integration tests use compile_commands.json, so generate it.
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
# LLVM 9.0 complains about this in older LLVM code, but it is fixed in newer versions, so we just ignore it.
if (NOT "${CMAKE_CXX_COMPILER_ID}" STREQUAL "AppleClang")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-defaulted-function-deleted")
endif()

###
# Setup LLVM+Clang includes and options.
find_package(LLVM REQUIRED CONFIG)
message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

include_directories(${LLVM_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})
include_directories("${LLVM_DIR}/../../../tools/clang/include")
include_directories("${LLVM_SRC_DIR}/tools/clang/include")

# Default system includes are not stored in compile_commands.json, but we need them for the integration tests as libclang will not use them automatically, so add them manually, then they are saved in compile_commands.json.
file(GLOB CMAKE_INCLUDE ${LLVM_DIR}/../../clang/*/include)
# Select only one directory. There should be only one.
list (GET CMAKE_INCLUDE 0 CMAKE_INCLUDE)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -idirafter ${CMAKE_INCLUDE}")

# Clang is compiled with -fvisibility=hidden. Compilers may generate weak symbols for template instantiations, which it does for the used clang templates inside of main.cc. This leads to weak symbols in main.o, which are visible by default (-fvisibility=default). Directly linking them to the library, which is the goal, triggers the warning that those weak symbols cannot be overridden at runtime, what might be desirable in other use-cases. Using -fvisibility=hidden hides the weak symbols and thus does not trigger the warning. (Maybe the reasoning is wrong... information are a bit sparse on this topic.)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fvisibility=hidden")
# Set flag -fno-rtti if LLVM was compiled with it (this is the default).
IF(NOT LLVM_ENABLE_RTTI)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-rtti")
ENDIF(NOT LLVM_ENABLE_RTTI)
IF(CMAKE_BUILD_TYPE MATCHES "Release")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D NDEBUG")
ELSEIF(CMAKE_BUILD_TYPE MATCHES "Debug")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wall")
ENDIF()
set(CMAKE_CXX_STANDARD 17)

find_package(ZLIB REQUIRED)

###
# Setup project specific things.
include_directories(src)
set(SOURCE_FILES src/FaultInjector.cpp src/SFIASTConsumer.cpp src/SFIAction.cpp src/utils.cpp src/FaultConstraints.cpp src/GenericRewriter.cpp)
file(GLOB HEADER_FILES src/*.h)
file(GLOB ALL_FAULTINJECTORS src/FaultInjectors/*.cpp)
file(GLOB DTL_ALL src/libs/dtl/dtl/*.hpp)

source_group("FaultInjectors" FILES ${ALL_FAULTINJECTORS} src/FaultInjectors/_all.h)
source_group("clang-sfi" FILES ${SOURCE_FILES} src/main.cpp )
source_group("libs" FILES src/libs/json.hpp ${DTL_ALL} ${FILESYSTEM_ALL})
if(WIN32)
	add_library(${PROJECT_NAME}_lib ${SOURCE_FILES} ${ALL_FAULTINJECTORS} ${HEADER_FILES} src/FaultInjectors/_all.h)
	add_executable(${PROJECT_NAME} src/main.cpp src/libs/json.hpp)
else()
	add_library(${PROJECT_NAME}_lib ${SOURCE_FILES} ${ALL_FAULTINJECTORS})
	add_executable(${PROJECT_NAME} src/main.cpp)
endif()
target_link_libraries(${PROJECT_NAME} PRIVATE ${PROJECT_NAME}_lib)
# Still experimental filesystem library needs flags.
if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
	target_link_libraries(${PROJECT_NAME} PRIVATE stdc++fs)
endif()
# And sometimes it is not available...
include_directories(${PROJECT_NAME} src/libs/filesystem/include/)

###
# Link clang libraries
# llvm_map_components_to_libnames does not work for clang libs, so we just link them all...
if(WIN32)
	file(GLOB CLANG_LIBS "${LLVM_DIR}/../../../${CMAKE_BUILD_TYPE}/lib/libclang*.lib")
else()
	file(GLOB CLANG_LIBS "${LLVM_DIR}/../../libclang*.a")
endif()
if (NOT CLANG_LIBS)
	message(FATAL_ERROR "Did not find any library files for \$CLANG_LIBS")
endif(NOT CLANG_LIBS)
# message(STATUS "Using the following Clang libs: ${CLANG_LIBS}")
if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "AppleClang")
	target_link_libraries(${PROJECT_NAME} PRIVATE ${CLANG_LIBS})
else()
	target_link_libraries(${PROJECT_NAME} PRIVATE -Wl,--start-group ${CLANG_LIBS} -Wl,--end-group)
endif()
# target_link_libraries(${PROJECT_NAME} PRIVATE clangBasic clangFrontend clangTooling clangToolingASTDiff clangToolingRefactor clangCrossTU clangIndex clangAST clangLex clangSerialization clangSema clangAnalysis clangARCMigrate clangEdit clangDriver clangParse)

# Find the libraries that correspond to the LLVM components that we wish to use
llvm_map_components_to_libnames(LLVM_LIBS Support core irreader Option ProfileData MC AArch64AsmParser)
# Link against LLVM libraries
target_link_libraries(${PROJECT_NAME} PRIVATE ${LLVM_LIBS})

###
# Some other libraries needed by LLVM/Clang
if(WIN32)
	target_link_libraries(${PROJECT_NAME} PRIVATE ${ZLIB_LIBRARIES} Version.lib)
else()
	target_link_libraries(${PROJECT_NAME} PRIVATE ${ZLIB_LIBRARIES} curses)
endif()
if(UNIX AND NOT APPLE)
	# Additional library for Linux systems.
	target_link_libraries(${PROJECT_NAME} PRIVATE pthread)
endif()

###
# Setup testing
#enable_testing()
include(CTest)
#add_subdirectory(googletest)
#add_subdirectory(unittests)
add_subdirectory(integrationtests)
