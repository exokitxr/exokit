@echo off
setlocal
set dir=%~dp0

"%dir%_curl.exe" --cacert "%dir%cacert.pem" %*
