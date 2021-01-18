# Introduction
This is a collection of tools I wrote for [Project DoomBox](https://twitter.com/dec_hl/status/931895013553922053)

They compile on [FreeDOS](http://www.freedos.org/) using [FreeBasic](https://www.freebasic.net/) and [DJGPP](http://www.delorie.com/djgpp/). I was to lazy to create 'real' Makefiles and compilation is done using simple DOS batch-files (make.bat).

# Tools overview
## ANSICAT
This small tool prints the contents of a randomly selected ANSI-file to the console. You have to specify a root-directory to search for a random file and ANSICAT will select and print one file below this directory. Printing is throttled.

In order to use ANSI escape sequences you have to load the FreeDOS ANSI driver. E.g. put the following line into your FDCONFIG.SYS:
```
DEVICEHIGH=C:\fdos\bin\nansi.sys
```

I use this with ANSI-Art from https://fuel.wtf/


### Usage
```
ANSICAT <directory>
```

## Menu (FreeBasic)
This is the first version of my game selector screen. It was written in FreeBasic, but the created EXE wouldn't run on the mainboard of the DoomBox. On all my other DOS computers it runs fine.

The program reads the file MENU.TXT, parses its entries and displays a list of options together with a BMP picture specific to the currently highlighted option.

By pressing ENTER the selected program can be started, ESC exits the program into the DOS prompt.

## Menu (C)
This is the second installment of the game selector screen. It was written in C using DJGPP and the [GRX](http://grx.gnu.de/) library. You have to compile GRX in order to build MENU.EXE.

In addition to the menu and the pictures it supports sound playback via a SoundBlaster compatible sound card. The sound code is taken from [shdon.com](http://www.shdon.com/dos/sound).
Because I'm a lazy bum (and the program was written for a single system) the SoundBlaster is hard-coded to P=220,I=5,D=1.

The sound files are from the [FreeDoom Project](https://freedoom.github.io/).

### MENU.TXT
The format of the file is simple (see examples):
* __"N=XXX"__ starts a new entry called "XXX". All following lines will be added to this entry.
* __"D=XXX"__ declares the directory to change to when this entry is selected.
* __"E=XXX"__ declares the program to start when this entry is selected.
* __"I=XXX"__ declares a BMP image to be shown when this entry is highlighted in the game slector.

### DOMENU.BAT
The data of the selected entry is written to EXE.OUT/DIR.OUT (FreeBasic version) or EFILE.TXT/DFILE.TXT (C version). The batch file DOMENU.BAT takes care of dealing with these files and calling the selected program.

*The batch file uses "SET/p" to read the selection from these output files. AFAIK this option is only available in FreeDOS.*

# License
## My code
All my code is released under [MIT License](https://opensource.org/licenses/MIT).

## Sound code from MENU_C
The sound code (dmagcc.c and dmagcc.h) is licensed under 
[Attribution 4.0 International (CC BY 4.0)](https://creativecommons.org/licenses/by/4.0/). 
The original code was taken from [Steven Don](http://www.shdon.com/dos/sound) and was modified to my purpose.

## WAV sounds in MENU_C
```
Copyright © 2001-2017 Contributors to the Freedoom project. All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.

    Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.

    Neither the name of the Freedoom project nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS “AS IS” AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

For a list of contributors to the Freedoom project, see the file CREDITS.
```
