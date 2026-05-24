#!/bin/bash
echo "=== Building for Linux ==="
mkdir -p build-linux && cd build-linux
cmake ..
make -j$(nproc)
echo "=== Done: build-linux/gdphysx ==="