rem @echo off
menu

if not exists exe.out goto end

set /p EXEC= <exe.out
set /p CHDIR= <dir.out
cdd %CHDIR%
%EXEC%

:end

