# Clang-SFI

This project utilizes [clang](https://clang.llvm.org/) to inject software faults into any C/C++ project.

## Prerequisites
LLVM/Clang is best built by LLVM/Clang itself. It also needs the libxml2 development headers. Further cmake and ninja-build are used.
```bash
sudo apt install libxml2 libxml2-dev
sudo apt install llvm
sudo apt install cmake ninja-build
```

If you want to compile with g++, you may encounter errors regarding the `std::__cxx11` namespace. This may be fixed by defining `_GLIBCXX_USE_CXX11_ABI`.

## Build

```bash
git clone --recursive <repository url> clang-sfi
cd clang-sfi
./setup-clang.sh
./build.sh
```

Make sure to use a linker (e.g. `ld.gold` or `lld` from clang), which does not care about the order of library parameters, or apply [another fix](https://stackoverflow.com/questions/34164594/gcc-ld-method-to-determine-link-order-of-static-libraries/34168951#34168951).

You may encounter memory issues when building LLVM, because parallel compilation can consume lots of memory. You may replace the call to `ninja` in `setup-clang.sh` with `ninja -j 1` to avoid parallelization.

## Run tests

Run `cd build && make && make check`.

To add additional integration tests, use `MFC_1` as template.

## Use

```bash
./clang-sfi <my_source_file.cpp>
```
