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

#include "fbgfx.bi"
#if __FB_LANG__ = "fb"
USING FB '' Scan code constants are stored in the FB namespace in lang FB
#endif

CONST NULL AS ANY PTR = 0

SUB ParseMenu(ItemName() AS STRING, ItemDir() AS STRING, ItemExec() AS STRING, ItemImage() AS STRING)
    OPEN "menu.txt" FOR INPUT AS #1
    IF ERR>0 THEN 
        PRINT "Can't find menu.txt"
    ELSE
        DIM AS STRING ln, dta, pre
        DIM AS INTEGER i = 0
        
        DO UNTIL( EOF(1) )          ' read file line by line
            LINE INPUT #1, ln
            ln = TRIM(ln)
            
            IF LEN(ln) > 0 THEN
                pre = UCASE(MID(ln, 1, 2))
                dta = MID(ln, 3)
                
                IF pre = "N=" THEN
                    i += 1
                    REDIM PRESERVE ItemName(i) AS STRING
                    REDIM PRESERVE ItemDir(i) AS STRING
                    REDIM PRESERVE ItemExec(i) AS STRING
                    REDIM PRESERVE ItemImage(i) AS STRING
                    
                    ItemName(i) = dta
                    ItemDir(i) = ""
                    ItemImage(i) = ""
                    ItemExec(i) = ""
                ELSEIF pre = "P=" THEN
                    ItemDir(i) = dta
                ELSEIF pre = "I=" THEN
                    ItemImage(i) = dta
                ELSEIF pre = "E=" THEN
                    ItemExec(i) = dta
                ELSE
                    PRINT "menu.txt error: " & ln
                END IF
            END IF
        LOOP
        CLOSE #1
    END IF
END SUB

FUNCTION bmp_load(BYREF filename AS CONST STRING) AS ANY PTR
    
    DIM AS INTEGER filenum, bmpwidth, bmpheight
    DIM AS ANY PTR img
    
    '' open BMP file
    filenum = FREEFILE()
    IF OPEN( filename FOR BINARY ACCESS READ AS #filenum ) <> 0 THEN RETURN NULL
    
    '' retrieve BMP dimensions
    GET #filenum, 19, bmpwidth
    GET #filenum, 23, bmpheight
    
    CLOSE #filenum
    
    '' create image with BMP dimensions
    img = IMAGECREATE( bmpwidth, ABS(bmpheight) )
    
    IF img = NULL THEN RETURN NULL
    
    '' load BMP file into image buffer
    IF BLOAD( filename, img ) <> 0 THEN IMAGEDESTROY( img ): RETURN NULL
    
    RETURN img
END FUNCTION

SUB RunStuff(ItemDir AS STRING, ItemExec AS STRING)
    DIM AS INTEGER d,e
    d = FREEFILE()
    e = FREEFILE()
    
    OPEN "dir.out" FOR OUTPUT AS #d
    PRINT #d, ItemDir
    CLOSE #d
    
    OPEN "exe.out" FOR OUTPUT AS #e
    PRINT #e, ItemExec
    CLOSE #e
    
END SUB

FUNCTION CreateFont() AS UBYTE PTR 
    '' Define character range
    CONST FIRSTCHAR = 32, LASTCHAR = 127
    
    CONST NUMCHARS = (LASTCHAR - FIRSTCHAR) + 1
    DIM AS UBYTE PTR p, myFont
    DIM AS INTEGER i
    
    '' Create custom font into PUT buffer
    
    myFont = IMAGECREATE(NUMCHARS * 8, 9)
    
    '' Put font header at start of pixel data
    
    IMAGEINFO( myFont, , , , , p )
    
    p[0] = 0
    p[1] = FIRSTCHAR
    p[2] = LASTCHAR
    
    '' PUT each character into the font and update width information
    FOR i = FIRSTCHAR TO LASTCHAR
        
        '' Here we could define a custom width for each letter, but for simplicity we use
        '' a fixed width of 8 since we are reusing the default font glyphs
        p[3 + i - FIRSTCHAR] = 8
        
        '' Create character onto custom font buffer by drawing using default font
        DRAW STRING myFont, ((i - FIRSTCHAR) * 8, 1), CHR(i), 32 + (i MOD 24) + 24
        
    NEXT i
    
    RETURN myFont
END FUNCTION


DIM ItemName() AS STRING
DIM ItemDir() AS STRING
DIM ItemExec() AS STRING
DIM ItemImage() AS STRING
DIM i AS INTEGER
DIM AS UBYTE PTR myFont

KILL("dir.out")
KILL("exe.out")

ParseMenu ItemName(), ItemDir(), ItemExec(), ItemImage()

IF LEN(ItemName) > 0 THEN
    DIM AS INTEGER current, old, release, doexit
    current = 1
    old = 0
    release = 1
    doexit = 0
    
    SCREENRES 640, 480, 8
    myFont = CreateFont()
    '' LoadImages ItemImage(), ItemBmp()
    
    DO
        IF old <> current THEN
            CLS
            DRAW STRING (80, 40), "[[[ MENU ]]] "
            FOR i = 1 TO UBOUND(ItemName)
                IF i = current THEN
                    DRAW STRING STEP (0, 20), ItemName(i), , myFont
                ELSE
                    DRAW STRING STEP (0, 20), ItemName(i)
                END IF
            NEXT
            IF LEN(ItemImage(current)) > 0 THEN
                DIM bmp AS ANY PTR
                bmp = bmp_load(ItemImage(current))
                IF bmp <> NULL THEN
                    PUT (320, 120), bmp
                    IMAGEDESTROY bmp
                END IF
            END IF
            old = current
            WHILE INKEY <> ""
            WEND
        END IF
        
        IF MULTIKEY(SC_UP   ) AND current>1 AND release=1 THEN 
            current = current - 1
            release = 0
            SLEEP 200
        ELSEIF MULTIKEY(SC_DOWN ) AND current<UBOUND(ItemName) AND release=1 THEN 
            current = current + 1
            release = 0
            SLEEP 200
        ELSE
            release = 1
            SLEEP 200
        END IF
        
        IF MULTIKEY(SC_ENTER) THEN 
            RunStuff ItemDir(current), ItemExec(current)
            IMAGEDESTROY myFont
            doexit = 1
        END IF
    LOOP UNTIL MULTIKEY(SC_ESCAPE) OR doexit=1
ELSE
    PRINT "No entries found in MENU.txt"
END IF
