cmake_minimum_required(VERSION 3.8)
set(PROJECT_NAME "clang-sfi")
project(${PROJECT_NAME})

###
# Setup LLVM+Clang includes and options.
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
find_package(LLVM REQUIRED CONFIG)
message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

include_directories(${LLVM_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})
include_directories("${LLVM_DIR}/../../../tools/clang/include")
include_directories("${LLVM_SRC_DIR}/tools/clang/include")

# Clang is compiled with -fvisibility=hidden. Compilers may generate weak symbols for template instantiations, which it does for the used clang templates inside of main.cc. This leads to weak symbols in main.o, which are visible by default (-fvisibility=default). Directly linking them to the library, which is the goal, triggers the warning that those weak symbols cannot be overridden at runtime, what might be desirable in other use-cases. Using -fvisibility=hidden hides the weak symbols and thus does not trigger the warning. (Maybe the reasoning is wrong... information are a bit sparse on this topic.)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fvisibility=hidden")
# Set flag -fno-rtti if LLVM was compiled with it (this is the default).
IF(NOT LLVM_ENABLE_RTTI)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-rtti")
ENDIF(NOT LLVM_ENABLE_RTTI)
set(CMAKE_CXX_STANDARD 14)

find_package(ZLIB REQUIRED)

###
# Setup project specific things.
include_directories(src)
set(SOURCE_FILES src/MatchHandler.cpp src/FaultInjector.cpp src/SFIASTConsumer.cpp src/SFIAction.cpp src/utils.cpp src/FaultConstraints.cpp)
file(GLOB HEADER_FILES src/*.h)
file(GLOB ALL_FAULTINJECTORS src/FaultInjectors/*.cpp)
file(GLOB DTL_ALL src/libs/dtl/dtl/*.hpp)

source_group("FaultInjectors" FILES ${ALL_FAULTINJECTORS} src/FaultInjectors/_all.h)
source_group("clang-sfi" FILES ${SOURCE_FILES} src/main.cpp )
source_group("libs" FILES src/libs/json.hpp ${DTL_ALL})
if(WIN32)
	add_library(${PROJECT_NAME}_lib ${SOURCE_FILES} ${ALL_FAULTINJECTORS} ${HEADER_FILES} src/FaultInjectors/_all.h)
	add_executable(${PROJECT_NAME} src/main.cpp src/libs/json.hpp)
else()
	add_library(${PROJECT_NAME}_lib ${SOURCE_FILES} ${ALL_FAULTINJECTORS})
	add_executable(${PROJECT_NAME} src/main.cpp)
endif()
target_link_libraries(${PROJECT_NAME} PRIVATE ${PROJECT_NAME}_lib)

###
# Find the libraries that correspond to the LLVM components
# that we wish to use

# Normally:
llvm_map_components_to_libnames(LLVM_LIBS Support core irreader Option ProfileData MC AArch64AsmParser)
# Link against LLVM libraries
#target_link_libraries(${PROJECT_NAME} PRIVATE ${LLVM_LIBS})
# but this does not work for the clang libs, so we just link against everything...
if(WIN32)
	file(GLOB CLANG_LIBS "${LLVM_DIR}/../../../Release/lib/libclang*.lib")
else()
	file(GLOB CLANG_LIBS "${LLVM_DIR}/../../libclang*.a")
endif()

if (NOT CLANG_LIBS)
	message(FATAL_ERROR "Did not find any library files in \"${LLVM_DIR}/../../*.a\"")
endif(NOT CLANG_LIBS)
# message(STATUS "Using the following Clang libs: ${CLANG_LIBS}")
# message(STATUS "Using the following LLVM libs: ${LLVM_LIBS}")
if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
	# GCC will likely use a linker which cares for link order, so we wrap the LLVM libs in a group.
	# message(STATUS "Using GCC so we fall back to --start-group/--end-group")
	target_link_libraries(${PROJECT_NAME} PRIVATE -Wl,--start-group ${CLANG_LIBS} ${LLVM_LIBS} -Wl,--end-group)
else()
	target_link_libraries(${PROJECT_NAME} PRIVATE ${CLANG_LIBS} ${LLVM_LIBS})
endif()
if(WIN32)
target_link_libraries(${PROJECT_NAME} PRIVATE ${ZLIB_LIBRARIES} Version.lib)
else()
target_link_libraries(${PROJECT_NAME} PRIVATE ${ZLIB_LIBRARIES} curses)
endif()
if(UNIX AND NOT APPLE)
	# Additional library for Linux systems.
	target_link_libraries(${PROJECT_NAME} PRIVATE pthread)
endif()
#target_link_libraries(${PROJECT_NAME} PRIVATE clangBasic clangFrontend clangTooling clangToolingASTDiff clangToolingRefactor clangCrossTU clangIndex clangAST clangLex clangSerialization clangSema clangAnalysis clangARCMigrate clangEdit clangDriver clangParse)


###
# Setup testing
#enable_testing()
include(CTest)
#add_subdirectory(googletest)
#add_subdirectory(unittests)
add_subdirectory(integrationtests)
