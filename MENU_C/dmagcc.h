/*
** Licensed under Attribution 4.0 International (CC BY 4.0) 
** https://creativecommons.org/licenses/by/4.0/
**
** Code was taken from http://www.shdon.com/dos/sound
** by Steven Don.
** This is a derived/modified version by Andre Seidelt <superilu@yahoo.com>
*/
#ifndef _dmagcc
#define _dmagcc

#include <stdbool.h>

struct WaveData {
    unsigned int SoundLength;
    unsigned char *Sample;
};

/****************************************************************************
** Checks to see if a Sound Blaster exists at a given address, returns     **
** true if Sound Blaster found, false if not.                              **
****************************************************************************/
extern bool ResetDSP(unsigned int a, unsigned int i, unsigned int d);

/****************************************************************************
** Send a byte to the DSP (Digital Signal Processor) on the Sound Blaster  **
****************************************************************************/
extern void WriteDSP(unsigned char Value);

/****************************************************************************
** Starts playback of the buffer. The DMA controller is programmed with    **
** a block length of 32K - the entire buffer. The DSP is instructed to     **
** play blocks of 8K and then generate an interrupt (which allows the      **
** program to clear the parts that have already been played)               **
****************************************************************************/
extern void StartPlayBack();

/****************************************************************************
** Clears an 8K part of the DMA buffer                                     **
****************************************************************************/
extern void ClearBuffer(unsigned int Buffer);

/****************************************************************************
** Mixes a sample with the contents of the DMA buffer                      **
****************************************************************************/
extern void MixVoice(struct WaveData *Voice);

/****************************************************************************
** Loads a wave file into memory. This procedure treats any file as a      **
** standard 11025Hz, 8bit, mono .WAV file. It doesn't perform any error    **
** checking.                                                               **
****************************************************************************/
extern bool LoadVoice(struct WaveData *Voice, char *FileName);

/****************************************************************************
** Converts a wave file so it can be mixed easily                          **
****************************************************************************/
extern void ConvertVoice(struct WaveData *Voice);

/****************************************************************************
** IRQ service routine - this is called when the DSP has finished playing  **
** a block                                                                 **
****************************************************************************/
extern void ServiceIRQ();

/****************************************************************************
** This procedure allocates 32K of memory to the DMA buffer and makes sure **
** that no page boundary is crossed                                        **
****************************************************************************/
extern void AssignBuffer();

/****************************************************************************
** Free the DMA buffer again                                               **
****************************************************************************/
extern void FreeBuffer();

#endif  // _dmagcc
