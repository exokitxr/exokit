mkdir out

set MLSDK="C:\Users\avaer\MagicLeap\mlsdk\v0.16.0"

rem call %MLSDK%\mabu.cmd -v -t release_host .\program-windows.mabu
rem call %MLSDK%\mabu.cmd -v -t release_host -s .\cert\app.privkey -p --create-package --allow-unsigned .\app-windows.package

call %MLSDK%\mabu.cmd -v -t debug_lumin .\program.mabu
call %MLSDK%\mabu.cmd -v -t debug_lumin -s .\cert\app.privkey -p --create-package --allow-unsigned .\app.package
