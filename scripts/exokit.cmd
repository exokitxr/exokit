@echo off

setlocal

set home=%~dp0
set node=%home%node\node.exe
set code=%home%index.js

IF EXIST "%node%" (
  "%node%" "%code%" %*
) ELSE (
  node "%code%" %*
)
