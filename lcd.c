#include <avr/io.h>
#include <inttypes.h>
#include <util/delay.h>
#include "lcd.h"

// NOTE: for 4-bit operation, bit0..3 must be pulled to 1 on the
// lcd device...
//
// Wiring:
// PB4          RS
// PB3          E
// PB2          R/#W
// PD7..0       DB7..0

// NOTE: PB7,6 are used for external crystal
// PB3..4 can be used for ICSP as well

#define RS          _BV(PB4)
#define E           _BV(PB3)
#define RW          _BV(PB2)

// T = width of E clk high/low states (> 250 ns) in cpu cycles
#if F_CPU < 4000000
#define T 1
#else
#define T (1 + ((F_CPU - 1) / 4000000))
#endif

const uint8_t hexchar[] = 
{
    '0', '1', '2', '3', '4', '5', '6', '7',
    '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'
};

void 
lcd_wait_busy(void) {
    uint8_t lcd_status;

    PORTB &= ~RS;
    DDRD = 0; // D[7..0] are inputs
    PORTB |= RW;

    do {
        _delay_loop_1(T);
        PORTB |= E;
        _delay_loop_1(T);
        lcd_status = PIND;
        PORTB &= ~E;
    } while (lcd_status & 0x80);

    PORTB &= ~RW;
    DDRD = 0xff; // D[7..0] are outputs
}

static void
e_pulse(uint8_t c) {
    PORTD = c;
    _delay_loop_1(T);
    PORTB |= E;
    _delay_loop_1(T);
    PORTB &= ~E;
}

void
lcd_cmd(uint8_t n)
{
    lcd_wait_busy();
    //PORTB &= ~RS; // lcd_wait_busy leaves RS at 0
    e_pulse(n);
}

void
lcd_data(uint8_t n)
{
    lcd_wait_busy();
    PORTB |= RS;
    e_pulse(n);
}

int
lcdwrite(char c, FILE *f) {
    lcd_data(c);
    return 0;
}

void
lcd_init(void)
{
    PORTB = 0;
    DDRB = RW | RS | E; // enable used PORT bits as output
    DDRD = 0xff;

    _delay_ms(15);  e_pulse(HD44780_CMD_FUNCTION_SET | HD44780_8_BIT);
    _delay_ms(5);   e_pulse(HD44780_CMD_FUNCTION_SET | HD44780_8_BIT);
    _delay_us(150); e_pulse(HD44780_CMD_FUNCTION_SET | HD44780_8_BIT);
    _delay_us(150); // not specified

    lcd_cmd(HD44780_CMD_FUNCTION_SET | HD44780_8_BIT | HD44780_2_LINES);

    // let printf just write to the lcd :)
    stdout = fdevopen(lcdwrite, NULL);
}

