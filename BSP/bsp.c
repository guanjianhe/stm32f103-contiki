#include "led.h"
#include "uart.h"


void bsp_init(void)
{
	/* 所有4位用于指定响应优先级,抢占优先级没有 */
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0);
	leds_init();
	uarts_init();	
}
