# How to install

## Windows
(Mac and Linux are not supported, Linux support is planned)

# 1. Install dependencies
`vcpkg install boost-uuid boost-json boost-dll sqlite3 magic-enum rang gtest --triplet x64-windows`

# 2. Building

Change `YOUR_VCPKG_DIR` to the path of your vcpkg folder.
```
git submodule update --init
mkdir build
cd build
cmake .. "-DCMAKE_TOOLCHAIN_FILE=YOUR_VCPKG_DIR\scripts\buildsystems\vcpkg.cmake"
cmake --build .
```
