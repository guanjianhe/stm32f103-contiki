#include "led.h"
#include "stm32f10x.h"

/* PA8,PD2 */
void leds_init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD, ENABLE);
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOD, &GPIO_InitStructure);

    leds_off(LED_RED);
    leds_off(LED_YELLOW);
}

void leds_on(unsigned char n)
{
    if (n == 0)
    {
        GPIOA->BRR = GPIO_Pin_8;
    }
    else if (n == 1)
    {
        GPIOD->BRR = GPIO_Pin_2;
    }
    else
    {
    }
}

void leds_off(unsigned char n)
{
    if (n == 0)
    {
        GPIOA->BSRR = GPIO_Pin_8;
    }
    else if (n == 1)
    {
        GPIOD->BSRR = GPIO_Pin_2;
    }
    else
    {
    }
}

void leds_toggle(unsigned char n)
{
    if (n == 0)
    {
        GPIOA->ODR ^= GPIO_Pin_8;
    }
    else if (n == 1)
    {
        GPIOD->ODR ^= GPIO_Pin_2;
    }
    else
    {
    }
}
