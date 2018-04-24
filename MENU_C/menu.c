/*
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
*/

#include <dos.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include "grx20.h"
#include "grxbmp.h"
#include "grxkeys.h"

#include "dmagcc.h"

#define MENU_FILE "MENU.TXT"
#define SND_START "dsbfg.WAV"
#define SND_ESC "dsbospn.WAV"
#define SND_ENTER "dskntsit.WAV"
#define SND_CHANGE "dspunch.WAV"

#define DIR_FILE "DFILE.TXT"
#define EXE_FILE "EFILE.TXT"

#define IDX_START 0
#define IDX_ESC 1
#define IDX_ENTER 2
#define IDX_CHANGE 3

#define SB_PORT 0x220
#define SB_IRQ 5
#define SB_DMA 1

#define DEBUG(x, ...)
//#define DEBUG(x, ...) printf(x, ##__VA_ARGS__)

#define WAVLOAD(idx, file)                  \
    if (LoadVoice(&VoiceData[idx], file)) { \
        ConvertVoice(&VoiceData[idx]);      \
    } else {                                \
        bailOut();                          \
    }

#define WAVPLAY(idx) MixVoice(&VoiceData[idx]);

GrTextOption opt;
struct WaveData VoiceData[4];  // Pointers to Wave files

typedef struct {
    char *txt;
    char *dir;
    char *exe;
    char *bmp;
    char *wav;
    GrPattern *img;
} menu_e;

char *loadFile(char *file_name) {
    // determine file size
    struct stat s;
    if (stat(file_name, &s)) {
        perror("stat()");
        return NULL;
    }
    int size = s.st_size;

    // get memory
    char *data = malloc(size + 2);
    if (!data) {
        perror("malloc()");
        return NULL;
    }

    // open file
    int fh = open(file_name, O_RDONLY | O_BINARY);

    if (fh < 0) {
        free(data);
        perror("open()");
        return NULL;
    }

    // read file
    int r = read(fh, data, size);

    DEBUG("Loaded %d bytes from '%s' to 0x%08X\n", r, file_name, (unsigned int)data);

    if (r != size) {
        perror("read()");
        close(fh);
        free(data);

        return NULL;
    } else {
        close(fh);
        data[size] = 0;  // add double null terminator to string
        data[size + 1] = 0;

        return data;
    }
}

menu_e *parseFile(char *data, int *num) {
    char *pos = data;

    // count number of entries by checking for lines that start with 'N'
    *num = 0;
    bool lastNewline = true;  // start of file counts as newline
    while (*pos) {
        if (*pos == 'N' && lastNewline) {
            (*num)++;
            lastNewline = false;
        } else if (*pos == '\n') {
            lastNewline = true;
        } else {
            lastNewline = false;
        }
        pos++;
    }

    if (*num == 0) {
        return NULL;
    }

    DEBUG("Found %d entries\n", *num);

    // malloc N=1 entries
    menu_e *entries = malloc((*num + 1) * sizeof(menu_e));

    DEBUG("Entries at 0x%08X\n", (unsigned int)entries);

    if (!entries) {
        return NULL;
    }

    int e = -1;
    pos = data;
    while (*pos) {
        DEBUG("found '%c'\n", *pos);
        if (*pos == 'N') {  // name
            // name found => new entry
            e++;
            pos += 2;
            entries[e].txt = pos;
            entries[e].dir = NULL;
            entries[e].exe = NULL;
            entries[e].bmp = NULL;
            entries[e].wav = NULL;
        } else if (e >= 0 && *pos == 'D') {  // dir
            pos += 2;
            entries[e].dir = pos;
        } else if (e >= 0 && *pos == 'I') {  // image
            pos += 2;
            entries[e].bmp = pos;
        } else if (e >= 0 && *pos == 'E') {  // exe
            pos += 2;
            entries[e].exe = pos;
        } else if (e >= 0 && *pos == 'W') {  // wav
            pos += 2;
            entries[e].wav = pos;
        }

        while (*pos) {
            // DEBUG("Skipping '%c'\n", *pos);
            pos++;
            if (*pos == '\n' || *pos == '\r') {
                *pos = 0;
                DEBUG("EOL found at 0x%08X\n", (unsigned int)pos);
                do {
                    pos++;
                } while (*pos == '\n' || *pos == '\r');
                break;
            }
        }

        DEBUG("Updated entry %d '%s'\n", e, entries[e].txt);
    }

    // add empty entry at end
    e++;
    entries[e].txt = NULL;
    entries[e].dir = NULL;
    entries[e].exe = NULL;
    entries[e].bmp = NULL;
    entries[e].wav = NULL;

    DEBUG("Parsing done at 0x%08X\n", (unsigned int)pos);

    return entries;
}

void writeFile(char *name, char *data) {
    char *newline = "\r\n";

    int fh = open(name, O_CREAT | O_WRONLY | O_BINARY);
    if (fh < 0) {
        perror("writeFile()");
    }
    write(fh, data, strlen(data));
    write(fh, newline, strlen(newline));

    close(fh);

    DEBUG("Wrote '%s' to '%s'\n", data, name);
}

void loadImages(menu_e *entries) {
    int e = 0;
    while (entries[e].txt) {
        entries[e].img = NULL;
        if (entries[e].bmp) {
            GrBmpImage *bmp = GrLoadBmpImage(entries[e].bmp);
            if (!bmp) {
                DEBUG("Could not find '%s'\n", entries[e].bmp);
            } else {
                DEBUG("Loaded image from '%s'\n", entries[e].bmp);
                DEBUG("Image is at 0x%08X, map at 0x%08X\n", (unsigned int)bmp, (unsigned int)bmp->bi_map);
                GrAllocBmpImageColors(bmp, NULL);
                entries[e].img = GrConvertBmpImageToStaticPattern(bmp);
                DEBUG("Patter is at 0x%08X\n", (unsigned int)entries[e].img);
            }
        }
        e++;
    }
}

void updateScreen(int cur, menu_e *entries, int num) {
    if (cur < 0 || cur >= num) {
        return;
    }

    //GrClearScreen(GrAllocColor(0, 0, 0));

    GrImageDisplay(320, 120, GrImageFromPattern(entries[cur].img));

    int factor = 480 / (num + 1);

    for (int i = 0; i < num; i++) {
        if (i == cur) {
            opt.txo_fgcolor.v = GrAllocColor(255, 0, 0);
        } else {
            opt.txo_fgcolor.v = GrAllocColor(128, 128, 128);
        }
        GrDrawString(entries[i].txt, strlen(entries[i].txt), 160, factor / 2 + factor * i, &opt);
    }
}

void sndExit() { FreeBuffer(); }

void bailOut() {
    sndExit();
    GrSetMode(GR_default_text);
    exit(0);
}

void sndInit() {
    // Check for Sound Blaster, address: 220h
    if (!ResetDSP(SB_PORT, SB_IRQ, SB_DMA)) {
        puts("SB init failed");
        bailOut();
    }

    // Load wave files and convert
    WAVLOAD(IDX_CHANGE, SND_CHANGE);
    WAVLOAD(IDX_ENTER, SND_ENTER);
    WAVLOAD(IDX_ESC, SND_ESC);
    WAVLOAD(IDX_START, SND_START);

    // Start playback
    StartPlayBack();
}

int main(int argc, char **argv) {
    int num;
    sndInit();
    GrSetMode(GR_width_height_bpp_graphics, 640, 480, 24);
    GrClearScreen(GrAllocColor(0, 0, 0));

    char *data = loadFile("menu.txt");
    menu_e *entries = parseFile(data, &num);
    if (!entries) {
        puts("Could not load entries");
        exit(1);
    }
    loadImages(entries);

    unlink(DIR_FILE);
    unlink(EXE_FILE);

    opt.txo_font = &GrFont_PC8x16;
    opt.txo_fgcolor.v = GrNOCOLOR;
    opt.txo_bgcolor.v = GrNOCOLOR;
    opt.txo_chrtype = GR_BYTE_TEXT;
    opt.txo_direct = GR_TEXT_DEFAULT;
    opt.txo_xalign = GR_ALIGN_CENTER;
    opt.txo_yalign = GR_ALIGN_CENTER;

    int cur = 0;
    updateScreen(cur, entries, num);
    WAVPLAY(IDX_START);
    while (true) {
        if (GrKeyPressed()) {
            GrKeyType key = GrKeyRead();
            switch (key) {
                case GrKey_Space:
                case GrKey_Return:
                    WAVPLAY(IDX_ENTER);
                    writeFile(DIR_FILE, entries[cur].dir);
                    writeFile(EXE_FILE, entries[cur].exe);
                    delay(600);
                    bailOut();
                    break;
                case GrKey_Escape:
                    WAVPLAY(IDX_ESC);
                    delay(1000);
                    bailOut();
                    break;
                case GrKey_Up:
                    if (cur > 0) {
                        cur--;
                    } else {
                        cur = num-1;
                    }
                    WAVPLAY(IDX_CHANGE);
                    updateScreen(cur, entries, num);
                    break;
                case GrKey_Down:
                    if (cur < num-1) {
                        cur++;
                    } else {
                        cur = 0;
                    }
                    WAVPLAY(IDX_CHANGE);
                    updateScreen(cur, entries, num);
                    break;
            }
            DEBUG("Entry '%s'\n", entries[cur].txt);
            DEBUG("  Dir '%s'\n", entries[cur].dir);
            DEBUG("  Exe '%s'\n", entries[cur].exe);
            DEBUG("  Bmp '%s'\n\n", entries[cur].bmp);
        }
    }
}
