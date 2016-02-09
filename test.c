/*
  $HeadURL: svn+ssh://gw/home/fules/svn/avr/stopwatch/trunk/stopwatch.c $
  $Rev: 351 $
  $Date: 2012-06-14 20:32:37 +0200 (Thu, 14 Jun 2012) $
  */
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <inttypes.h>
#include <util/delay.h>
#include <stdio.h>
#include <math.h>
#include "lcd.h"

volatile uint32_t tick = 0;
volatile uint8_t progress = 0, sensors = 0;

const uint8_t userchars[] =
{
    0x10,  0x10,  0x10,  0x10,  0x10,  0x10,  0x10,  0x10, 
    0x08,  0x08,  0x08,  0x08,  0x08,  0x08,  0x08,  0x08, 
    0x04,  0x04,  0x04,  0x04,  0x04,  0x04,  0x04,  0x04, 
    0x02,  0x02,  0x02,  0x02,  0x02,  0x02,  0x02,  0x02, 
    0x01,  0x01,  0x01,  0x01,  0x01,  0x01,  0x01,  0x01, 
};
void
lcd_setup(void) {
    uint8_t i;

    // lcd setup
    lcd_cmd(HD44780_CMD_DISPLAY_CONTROL);
    lcd_cmd(HD44780_CMD_ENTRY_MODE_SET | HD44780_INCREMENT_CURSOR);
    lcd_cmd(HD44780_CMD_SHIFT_CONTROL);
    lcd_cmd(HD44780_CMD_CLEAR_DISPLAY);
    lcd_cmd(HD44780_CMD_DISPLAY_CONTROL | HD44780_DISPLAY_ON);
    //lcd_cmd(HD44780_CMD_DISPLAY_CONTROL | HD44780_DISPLAY_ON | HD44780_CURSOR_ON | HD44780_BLINK_ON);

    // set up the progress indicator chars to 0..4
    lcd_cmd(HD44780_CMD_SET_CGRAM_ADDR | 0x00);
    for (i = 0; i < sizeof(userchars); i++)
        lcd_data(userchars[i]);
    lcd_cmd(HD44780_CMD_SET_DDRAM_ADDR | 0x00);
}

ISR(TIMER1_COMPA_vect) {
    progress++;
    if (progress >= 16 * 5)
        progress = 0;
    tick++;
}

int
main(void)
{
    sei();
    lcd_init();
    lcd_setup();

    // set up timer1 for a 0.1-sec interrupt
    TCCR1B = _BV(WGM12) | 5; // ctc mode up to OCR1A, clk = sysclk / 1024
    OCR1A = F_CPU / 1024 / 10; // 10 Hz
    TCNT1 = 0; // timer reset
    TIMSK1 = _BV(OCIE1A);

    DDRC |= _BV(PC5);

    set_sleep_mode(SLEEP_MODE_IDLE);
    lcd_cmd(HD44780_CMD_CLEAR_DISPLAY);
#if HD44780_BIT == 8
    printf("8bit at last");
#elif HD44780_BIT == 4
    printf("4bit still");
#else
#error "Oops..."
#endif
    while (1) {
        uint8_t i, p;
        sleep_mode();
        PORTC ^= _BV(PC5);
        lcd_cmd(HD44780_CMD_SET_DDRAM_ADDR | 0x40);
        p = progress / 5;
        for (i = 0; i < 16; i++)
            lcd_data((i != p) ? ' ' : progress % 5);
    }
    return 0;
}
