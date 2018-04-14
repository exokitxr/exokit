@echo on
setlocal
SET INNOSETUP=%CD%\exokit.iss
SET ORIG=%CD%

node -p -e "require('./package.json').version" > version.txt
set /p version=<version.txt
echo #define MyAppVersion "%version%" > version.iss

REM Get the version number from the setup file
REM for /f "tokens=*" %%i in ('findstr /n . %INNOSETUP% ^| findstr ^4:#define') do set L=%%i
REM set version=%L:~24,-1%

REM Get the version number from the core executable
REM for /f "tokens=*" %%i in ('findstr /n . %GOPATH%\nvm.go ^| findstr ^NvmVersion^| findstr ^21^') do set L=%%i
REM set goversion=%L:~19,-1%

REM IF NOT %version%==%goversion% GOTO VERSIONMISMATCH

SET DIST=%CD%\dist\%version%

REM Build the executable
echo Building Exokit Browser for Windows
echo "=========================================>"

REM Clean the dist directory
rmdir /S /Q "%DIST%"
mkdir "%DIST%"

echo Creating distribution in %DIST%

echo "Building Exokit...."
set PATH=%CD%\node;%PATH%
CALL npm cache clean --force
CALL rmdir /S /Q node_modules
CALL npm i

REM echo Building "noinstall" zip...
REM for /d %%a in (%GOBIN%) do (buildtools\zip -j -9 -r "%DIST%\nvm-noinstall.zip" "%CD%\LICENSE" "%%a\*" -x "%GOBIN%\nodejs.ico")

echo "Building the primary installer..."
buildtools\iscc %INNOSETUP% /o%DIST%
buildtools\zip -j -9 -r "%DIST%\exokit-setup.zip" "%DIST%\exokit-setup.exe"

REM echo "Generating Checksums for release files..."
REM 
REM for /r %i in (*.zip *.exe) do checksum -file %i >> %i.sha256.txt
REM echo "Distribution created. Now cleaning up...."
REM del %GOBIN%/nvm.exe

echo "Done."
@echo on
