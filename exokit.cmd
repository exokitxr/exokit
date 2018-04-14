setlocal
set home=%~dp0
set isolator=%home%node_modules\isolator\lib\windows\isolator.exe
set node=%home%node\node.exe
set code=%home%index.js

IF EXIST "%node%" (
  "%isolator%" -- "%node%" "%code%" %*
) ELSE (
  "%isolator%" -- node "%code%" %*
)
