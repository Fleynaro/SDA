# How to install

## Windows
(Mac and Linux are not supported, Linux support is planned)

# 1. Install dependencies
`vcpkg install nlohmann-json magic-enum sqlitecpp gtest zydis asmjit thrift minhook imgui --triplet x64-windows`
You can install the dependencies however you like as long as CMake finds them.

# 2. Building

# WIP
```
mkdir build
cd build
cmake .. -G "Visual Studio 16 2019" -A x64 "-DCMAKE_TOOLCHAIN_FILE=C:\src\vcpkg\scripts\buildsystems\vcpkg.cmake"
cmake --build .
```
