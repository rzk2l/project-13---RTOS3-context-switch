#ifndef __MIROS_H
#define __MIROS_H
#include <stdint.h>


typedef struct {
    uint32_t* sp;
} OSThread;

typedef void(*OSThreadHandler)();   //THIS MEANS THAT OSThreadHandler IS FROM NOW ON A POINTER TO A FUNCTION THAT TAKES NO ARGUMENTS AND RETURNS NOTHING

void OSInit();

void OSRun();

// OSSched MUST BE CALLED WHEN INTERRUPTS ARE DISABLED
void OSSched();

void OSThreadStart(OSThread* me, OSThreadHandler threadHandler, void* stackMem, uint32_t stackSize);

#endif