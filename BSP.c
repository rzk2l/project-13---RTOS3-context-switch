/* Board Support Package */
#include <stm32f401xe.h>
#include "BSP.h"
#include "miros.h"

static volatile uint32_t l_tickCtr;

void assert_failed(){
    NVIC_SystemReset();
}

void BSP_init(){
  	SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk | SysTick_CTRL_TICKINT_Msk | SysTick_CTRL_ENABLE_Msk;
	SysTick->VAL = 0;
	SysTick->LOAD = (SYS_CLOCK_FREQ/BSP_TICKS_PER_SEC) - 1;
	
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;

	GPIOA->MODER |= GPIO_MODER_MODER5_0;
}

void SysTick_Handler(){
    ++l_tickCtr;

	__disable_irq();
	OSSched();
	__enable_irq();
}

uint32_t BSP_tickCtr(){
	uint32_t tickCtr;

	__disable_irq();	//AVOID RACE CONDITION IN CASE INTERRUPT FIRES AT THIS EXACT MOMENT
	tickCtr = l_tickCtr;
	__enable_irq();

	return l_tickCtr;
}

void BSP_delay(uint32_t ticks){
	uint32_t start = BSP_tickCtr();
	while ((BSP_tickCtr() - start) < ticks){
	}
}