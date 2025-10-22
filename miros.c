#include "miros.h"
#include <stm32f401xe.h>

OSThread* volatile OS_curr;
OSThread* volatile OS_next;

OSThread* OSThreads[32+1]; // ARRAY OF THREADS THAT HAVE BEEN STARTED SO FAR, IT SAVES POINTERS TO THREADS
uint8_t OSThreadNumber;		// HOW MANY THREADS HAVE BEEN STARTED SO FAR
uint8_t OS_currIndex;		// HOLDS THE ARRAY INDEX OF THE CURRENT THREAD RUNNING 

void OSInit(){
    SCB->SHPR[10] = 0xFF; //SETS PENDSV PRIORITY TO LOW
}

void OSRun(){
	__disable_irq();
	OSSched();
	__enable_irq();
	// CODE AFTER HERE WILL NEVER RUN BECAUSE PENDSV INTERRUPTS HERE AND WILL GO TO THE THREADS AND NEVER RETURN HERE
}

void OSSched(){

	if ((SCB->ICSR & SCB_ICSR_PENDSVSET_Msk) != 0U) {
        return;
    }
	++OS_currIndex;
	if(OS_currIndex == OSThreadNumber){
		OS_currIndex = 0;
	} 
	OS_next = OSThreads[OS_currIndex];

    if(OS_next != OS_curr){
        SCB->ICSR |= SCB_ICSR_PENDSVSET_Msk;
    }
     
}

void OSThreadStart(OSThread* me, OSThreadHandler threadHandler, void* stackMem, uint32_t stackSize){
    uint32_t* pStack = (uint32_t*)((((uint32_t)stackMem + stackSize)/8)*8);    //TAKES THE ADDRESS AT THE TOP OF THE PRIVATE STACK AND THE PRIVATE STACK'S SIZE AND CALCULATES THE VALUE OF THE STACK POINTER    
  	*(--pStack) = 0x00000000;  // reserved word
	*(--pStack) = 0x00000000;  // FPSCR
	*(--pStack) = 0x41410000;  // S15 (example)
	*(--pStack) = 0x41400000;  // S14
	*(--pStack) = 0x41300000;  // S13
	*(--pStack) = 0x41200000;  // S12
	*(--pStack) = 0x41100000;  // S11
	*(--pStack) = 0x41000000;  // S10
	*(--pStack) = 0x40F00000;  // S9
	*(--pStack) = 0x40E00000;  // S8
	*(--pStack) = 0x40D00000;  // S7
	*(--pStack) = 0x40C00000;  // S6
	*(--pStack) = 0x40B00000;  // S5
	*(--pStack) = 0x40A00000;  // S4
	*(--pStack) = 0x40900000;  // S3
	*(--pStack) = 0x40800000;  // S2
	*(--pStack) = 0x40700000;  // S1
	*(--pStack) = 0x40600000;  // S0
	*(--pStack) = (1 << 24); //xPSR //decrement to reach the first free entry in the stack
	*(--pStack) = (uint32_t)threadHandler;  //PC
	*(--pStack) = 0x0; //LR
	*(--pStack) = 0x12; //R12
	*(--pStack) = 0x3; //R3
	*(--pStack) = 0x2; //R2
	*(--pStack) = 0x1; //R1
	*(--pStack) = 0x0; //R0
    // ADITIONAL FAKE REGISTERS R4-R11
	*(--pStack) = 0xB; //R11
	*(--pStack) = 0xA; //R10
	*(--pStack) = 0x9; //R9
	*(--pStack) = 0x8; //R8
	*(--pStack) = 0x7; //R7
	*(--pStack) = 0x6; //R6
  *(--pStack) = 0x5; //R5
	*(--pStack) = 0x4; //R4

    me->sp = pStack;	//ASSIGNS THE TOP OF THE STACK TO THE THREAD STARTED

	OSThreads[OSThreadNumber] = me;	// ADDS THE POINTER TO THE STARTED THREAD TO THE ARRAY OF THREAD POINTERS, this array will be traversed by the scheduler
	++OSThreadNumber;
}

void PendSV_Handler(){
	__asm volatile (

	/* __disable_irq(); */

	"CPSID         I\n"
    /* if(OS_curr != (OSThread*)0){ */ 

	"LDR     r0, =OS_curr\n"
        "LDR     r0, [r0, #0x00]\n"     // r0 = OS_curr
        "CBZ     r0, PendSV_restore\n"  // Skip save if OS_curr is NULL
        
        "PUSH    {r4-r11}\n"
        "STR     sp, [r0, #0x00]\n"     // OS_curr->sp = sp

    "PendSV_restore:\n"
        // 2. Restore context of the next thread (OS_next)
        "LDR     r1, =OS_next\n"        // r1 = &OS_next
        "LDR     r1, [r1, #0x00]\n"     // r1 = OS_next (e.g., &blink2)
        "LDR     sp, [r1, #0x00]\n"     // sp = OS_next->sp
        "POP     {r4-r11}\n"            // Restore r4-r11 from new stack
        
        // 3. Update OS_curr = OS_next
        "LDR     r0, =OS_curr\n"        // r0 = &OS_curr
        "STR     r1, [r0, #0x00]\n"     // *(&OS_curr) = r1 (OS_curr = OS_next)

        "CPSIE   I\n"
        "BX      lr\n" 
);
}