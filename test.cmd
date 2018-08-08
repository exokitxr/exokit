call npm run asbuild

mkdir out

call ..\wabt\build\Debug\wasm2c build\optimized.wasm >out\main.c

for %%I in (..\wabt\wasm2c\wasm-rt.h ..\wabt\wasm2c\wasm-rt-impl.h ..\wabt\wasm2c\wasm-rt-impl.c include\wasm.h config\manifest.xml config\program.mabu config\app.package) do copy %%I out

call node postprocess.js

set MLSDK="C:\Users\avaer\MagicLeap\mlsdk\v0.16.0"
echo calling "%MLSDK%\mabu.cmd"
call %MLSDK%\mabu.cmd .\out\program.mabu
call %MLSDK%\mabu.cmd -p --create-package --allow-unsigned .\out\app.package
