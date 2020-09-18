#!/bin/sh -l

set -x

# Install
make install

# Compile examples
cd ../examples
mkdir build
cd build
cmake ..
make

# return to the compilation place
cd ../../build
