/'
MIT License

Copyright (c) 2018 Andre Seidelt <superilu@yahoo.com>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:
The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
'/

#include "dir.bi"

CONST attr_mask = fbNormal OR fbDirectory

' create a list of all files below a directory
' IN 
'   file := the directory to recursively process for files
' INOUT
'   Array := the array to append newly found files to
SUB AllFiles(file AS STRING, Array() AS STRING)
    DIM AS STRING s
    DIM AS INTEGER attr, i
    
    i = UBOUND(Array)
    s = DIR(file & "\*", attr_mask, @attr)
    DO UNTIL s = ""
        IF s <> "." AND s <> ".." THEN
            IF (attr AND fbDirectory) <> 0 THEN
                AllFiles file & "\" & s, Array()
            ELSE
                IF (attr AND fbHidden) = 0 THEN
                    i += 1
                    REDIM PRESERVE Array(i)
                    Array(i) = file & "\" & s
                END IF
            END IF
        END IF
        s = DIR("", attr_mask, @attr)
    LOOP
END SUB

' select an entry randomly from an array
' IN
'   files := an array of strings
' OUT
'   sel := the entry randomly selected
SUB RandomFile(files() AS STRING, sel AS STRING)
    DIM i AS INTEGER
    i = INT(RND * UBOUND(files)) + 1
    sel = files(i)
END SUB

' main entry point
DIM AS STRING arg = COMMAND(1)
IF LEN(arg) = 0 THEN
    PRINT "Usage:"
    PRINT "    " & COMMAND(0) & " <directory>"
ELSE
    DIM Files(0) AS STRING
    DIM sel AS STRING
    
    RANDOMIZE ,3                ' initialize random generator
    
    AllFiles arg, Files()       ' scan diretory from command line for all files
    RandomFile Files(), sel     ' randomly select a file
    PRINT sel                   ' print name
    
    OPEN sel FOR INPUT AS #1    ' open file
    OPEN CONS FOR OUTPUT AS #2  ' open console
    
    DIM AS STRING ln
    
    DO UNTIL( EOF(1) )          ' copy file line by line to console with a short delay
        LINE INPUT #1, ln
        PRINT #2, ln
        SLEEP 60
    LOOP
    
    CLOSE #1                    ' close files
    CLOSE #2
END IF

