@echo on
SET INNOSETUP=%CD%\exokit.iss
SET ORIG=%CD%
SET version=0.0.128

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
REM 
REM echo "Building the primary installer..."
REM buildtools\iscc %INNOSETUP% /o%DIST%
REM buildtools\zip -j -9 -r "%DIST%\nvm-setup.zip" "%DIST%\nvm-setup.exe"
REM echo "Generating Checksums for release files..."
REM 
REM for /r %i in (*.zip *.exe) do checksum -file %i >> %i.sha256.txt
REM echo "Distribution created. Now cleaning up...."
REM del %GOBIN%/nvm.exe
REM 
echo "Done."
@echo on
