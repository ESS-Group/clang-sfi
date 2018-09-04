# Clang-SFI
This project utilizes [clang](https://clang.llvm.org/) to inject software faults into any C/C++ project.

## Build
Change `LLVM_DIR` to point to an LLVM installation.

Run `mkdir build && cd build && cmake -G "Unix Makefiles" && make -j`.
