#include "led.h"
#include "uart.h"


void bsp_init(void)
{
	/* ����4λ����ָ����Ӧ���ȼ�,��ռ���ȼ�û�� */
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0);
	leds_init();
	uarts_init();	
}
