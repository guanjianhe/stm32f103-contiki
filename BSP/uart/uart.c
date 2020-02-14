#include "uart.h"

#define UART1_TXB 128

static volatile struct
{
    uint16_t tri, twi, tct;
    uint8_t tbuf[UART1_TXB];
} Fifo1;

void xputc(uint8_t d)
{
    int i;

    while (Fifo1.tct >= UART1_TXB) ;

    i = Fifo1.twi;
    Fifo1.tbuf[i] = d;
    Fifo1.twi = ++i % UART1_TXB;
    __disable_irq();
    Fifo1.tct++;
    USART_ITConfig(USART1, USART_IT_TXE, ENABLE);
    __enable_irq();
}




static void uart1_init(uint32_t baudrate)
{
    NVIC_InitTypeDef NVIC_InitStructure;
    GPIO_InitTypeDef GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_AFIO, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
    //NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    USART_InitStructure.USART_BaudRate = baudrate;
    /* 数据位8bit */
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    /* 停止位1bit */
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    /* 无奇偶校验 */
    USART_InitStructure.USART_Parity = USART_Parity_No ;
    /* 无硬件流控制 */
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    /* 收发模式 */
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    USART_Init(USART1, &USART_InitStructure);

    /* 使能接收中断 */
    USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
    USART_Cmd(USART1, ENABLE);
    USART_ClearFlag(USART1, USART_FLAG_TC);
}

void uarts_init(void)
{
    uart1_init(115200);
}

void USART1_IRQHandler(void)
{
	uint8_t d;
    int i;

    if (USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)
    {
        d = USART_ReceiveData(USART1);
        xputc(d);
    }

    if (USART_GetITStatus(USART1, USART_IT_TXE) != RESET)
    {
        i = Fifo1.tct;

        if (i--)
        {
            Fifo1.tct = (unsigned short)i;
            i = Fifo1.tri;
            USART_SendData(USART1, Fifo1.tbuf[i]);
            Fifo1.tri = ++i % UART1_TXB;
        }
        else
        {
            USART_ITConfig(USART1, USART_IT_TXE, DISABLE);
        }
    }
}

#if 1

#include "stdio.h"

int fputc(int ch, FILE* f)
{
	xputc((uint8_t)ch);
	#if 0
    /* 发送一个字节数据到串口 */
    USART_SendData(USART1, (unsigned char) ch);

    /* 等待发送完成，注意USART_FLAG_TXE不能改成USART_FLAG_TC
    否则有可能失去首字母 */
    while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);
	#endif

    return (ch);
}
#endif

