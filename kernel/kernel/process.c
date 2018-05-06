#include "x86.h"
#include "device.h"
#include<elf.h>
extern struct ProcessTable PCB[2];
extern struct Tss tss;

void initProc()
{
    PCB[0].state = RUNABLED;
    PCB[1].state = DEAD;
    PCB[0].timeCount = 1;
    PCB[0].sleeptime = 0;
    PCB[0].pid = 1;
}