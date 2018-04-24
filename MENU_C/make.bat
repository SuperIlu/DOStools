mode co80,50
del menu.o
del menu.exe

rem set PATH=c:\devel\djgpp\bin;%PATH%

gcc -Wall -pedantic -g -c -o menu.o menu.c -Ic:\devel\djgpp\contrib\grx249\include -Ic:\devel\djgpp\contrib\grx249\src\include
gcc -Wall -pedantic -g -c -o dmagcc.o dmagcc.c -Ic:\devel\djgpp\contrib\grx249\include -Ic:\devel\djgpp\contrib\grx249\src\include
gcc -g -o menu.exe menu.o dmagcc.o -Lc:\devel\djgpp\contrib\grx249\lib\dj2 -lgrx20

