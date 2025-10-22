#ifndef __BSP_H
#define __BSP_H

#define SYS_CLOCK_FREQ 16000000
#define BSP_TICKS_PER_SEC 100

void assert_failed();
void BSP_init();
uint32_t BSP_tickCtr();
void BSP_delay(uint32_t ticks);



#endif