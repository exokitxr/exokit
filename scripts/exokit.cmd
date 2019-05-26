setlocal

set home=%~dp0
set node=%home%..\node\node.exe
set code=%home%..\src\index.js

"%node%" "%home%urlcheck.js" %* > temp.txt
set /p site=<temp.txt


IF EXIST "%node%" (
  "%node%" "%code%" "%site%"
) ELSE (
  node "%code%" "%site%"
)
