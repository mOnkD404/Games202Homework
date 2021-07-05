#!/bin/bash
rm -rf build
mkdir build
cd build
cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=1 ..
make -j8
# ./lut-Emu-MC
# ./lut-Eavg-MC
./lut-Emu-IS
./lut-Eavg-IS
cd ..
