@echo off
setlocal
cd "%~dp0"
node_modules\.bin\npx node . %*
