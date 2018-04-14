@echo on

setlocal

SET INNOSETUP=%CD%\exokit.iss

set version=0.0.1
echo #define MyAppVersion "%version%" > version.iss

SET DIST=%CD%\dist\%version%

REM Clean the dist directory
rmdir /S /Q "%DIST%"
mkdir "%DIST%"

echo "Building the primary installer..."
buildtools\iscc %INNOSETUP% /o%DIST%

echo "Done."
@echo on
