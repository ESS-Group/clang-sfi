if [ $# -eq 2 ]; then
	# $2 is CMAKE_CURRENT_SOURCE_DIR
	# $3 is CMAKE_CURRENT_BINARY_DIR
	SOURCE_DIR="$2"
	BUILD_DIR="$3"
else
	SOURCE_DIR="$(dirname "$0")"
	BUILD_DIR="$(dirname "$0")"
fi

EXECUTABLE="$(dirname "$0")/../../build/clang-sfi"

