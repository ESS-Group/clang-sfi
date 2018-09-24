# Clang-SFI
This project utilizes [clang](https://clang.llvm.org/) to inject software faults into any C/C++ project.

## Build
```
git clone --recursive <repository url> clang-sfi
cd clang-sfi
./setup-clang.sh
./build.sh
```

Make sure to use a linker (e.g. `ld.gold` or `lld` from clang), which does not care about the order of library parameters, or apply [another fix](https://stackoverflow.com/questions/34164594/gcc-ld-method-to-determine-link-order-of-static-libraries/34168951#34168951).

## Run tests
Run `cd build && make && make check`.

To add additional integration tests, use `MFC_1` as template.

## Use
