setlocal

set home=%~dp0
set node=%home%..\node\node.exe
set code=%home%..\src\index.js

IF EXIST "%node%" (
  "%node%" "--experimental-worker" "%code%" %*
) ELSE (
  node "--experimental-worker" "%code%" %*
)
