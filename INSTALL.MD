# How to install

## Windows
(Mac and Linux are not supported, Linux support is planned)

# 1. Install dependencies
```
vcpkg install boost-uuid boost-json boost-dll sqlite3 magic-enum v8 rang gtest --triplet x64-windows
npm install -g cmake-js
```

# 2. Building

Change `YOUR_VCPKG_DIR` to the path of your vcpkg folder.
```
git submodule update --init
mkdir build
cd build
cmake .. "-DCMAKE_TOOLCHAIN_FILE=YOUR_VCPKG_DIR\scripts\buildsystems\vcpkg.cmake"
cmake-js configure --runtime=electron --runtime-version=20.2.0
cmake --build .
```
