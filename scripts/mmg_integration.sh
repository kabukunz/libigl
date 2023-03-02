#!/bin/bash

rm -rf build/CMakeCache.txt
rm -rf build/ExternalProjects/mmg/build/CMakeCache.txt

cmake -S . -B build -G Ninja \
-DCMAKE_BUILD_TYPE=Release \
-DLIBIGL_BUILD_TESTS=OFF \
-DLIBIGL_BUILD_TUTORIALS=OFF \
-DLIBIGL_USE_STATIC_LIBRARY=OFF \
-DLIBIGL_WITH_MMG=ON

cmake --build build
