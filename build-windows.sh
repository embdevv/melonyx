#!/bin/bash
echo "=== Building for Windows ==="
mkdir -p build-windows && cd build-windows
cmake .. -DCMAKE_TOOLCHAIN_FILE=../cmake/windows-toolchain.cmake
make -j$(nproc)
echo "=== Done: build-windows/gdphysx.exe ==="