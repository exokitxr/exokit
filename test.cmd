mkdir out

set MLSDK="C:\Users\avaer\MagicLeap\mlsdk\v0.16.0"

call %MLSDK%\mabu.cmd -t debug_lumin .\out\program.mabu
call %MLSDK%\mabu.cmd -t debug_lumin -s .\cert\app.privkey -p --create-package --allow-unsigned .\out\app.package
