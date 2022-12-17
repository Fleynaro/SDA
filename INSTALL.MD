# How to install

## Windows
(Mac and Linux are not supported, Linux support is planned)

# 1. Install dependencies
You need install Vcpkg, Node.js and Visual Studio (Windows SDK 10).
```
vcpkg install boost-uuid boost-json boost-dll sqlite3 magic-enum rang gtest --triplet x64-windows
npm install -g cmake-js
```

# 2. Building

Change `YOUR_VCPKG_DIR` to the path of your vcpkg folder.
```
git submodule update --init
mkdir build
cd build
cmake -T host=x64 -A x64 .. "-DCMAKE_TOOLCHAIN_FILE=YOUR_VCPKG_DIR\scripts\buildsystems\vcpkg.cmake"
cd ..
cmake-js configure --runtime=electron --runtime-version=20.2.0
cmake --build .
```
