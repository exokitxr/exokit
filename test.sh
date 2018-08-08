#!/bin/bash

npm run asbuild
mkdir -p out
../wabt/bin/wasm2c build/optimized.wasm >out/main.c
cp ../wabt/wasm2c/*.{h,c} out/
sed -i -e 's/static u32 main(/u32 main(/g' out/main.c
cp include/wasm.h out/wasm.h
gcc out/main.c out/wasm-rt-impl.c -o site
./site
