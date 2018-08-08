call npm run asbuild

mkdir out

call ..\wabt\build\Debug\wasm2c build\optimized.wasm >out\main.c

for %%I in (..\wabt\wasm2c\wasm-rt.h ..\wabt\wasm2c\wasm-rt-impl.h ..\wabt\wasm2c\wasm-rt-impl.c include\wasm.h) do copy %%I out

call node postprocess.js

call cl /W4 out\main.c out\wasm-rt-impl.c /link /out:site.exe

.\site.exe
