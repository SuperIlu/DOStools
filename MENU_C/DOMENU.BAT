rem @echo off
menu

if not exists exe.out goto end

set /p EXEC= <efile.txt
set /p CHDIR= <dfile.txt
cdd %CHDIR%
%EXEC%

:end

