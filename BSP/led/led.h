#ifndef _LED_H
#define _LED_H

void leds_init(void);
void leds_on(unsigned char n);
void leds_off(unsigned char n);
void leds_toggle(unsigned char n);

enum
{
    LED_RED,
    LED_YELLOW
};

#endif
