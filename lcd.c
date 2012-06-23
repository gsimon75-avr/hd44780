#include <avr/io.h>
#include <inttypes.h>
#include <util/delay.h>
#include "lcd.h"

// NOTE: for 4-bit operation, bit0..3 must be pulled to 1 on the
// lcd device...
//
// Wiring:
// PB1          RS
// PB0          E
// PB5..2       DB7..4
// PD7          R/#W
// Vcc -4k7-    DB3..0  (PULL THEM UP OR IT WON'T WORK!)

// NOTE: PB7,6 are used for external crystal
// PB3..4 can be used for ICSP as well

#define RS          _BV(PB1)
#define E           _BV(PB0)
#define RW          _BV(PD7)

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
    DDRB &= 0xc3; // B[2..5] are inputs
    PORTD |= RW;

    do {
        _delay_loop_1(T);
        PORTB |= E;
        _delay_loop_1(T);
        lcd_status = (PINB << 2) & 0xf0;
        PORTB &= ~E;

        _delay_loop_1(T);
        PORTB |= E;
        _delay_loop_1(T);
        PORTB &= ~E;
    } while (lcd_status & 0x80);

    PORTD &= ~RW;
    DDRB |= 0x3c; // B[2..5] are outputs
}

void
lcd_cmd(uint8_t n)
{
    lcd_wait_busy();

    _delay_loop_1(T);
    PORTB = ((n & 0xf0) >> 2) | E;   /* RS=0 */
    _delay_loop_1(T);
    PORTB &= ~E;
    
    _delay_loop_1(T);
    PORTB = ((n & 0x0f) << 2) | E;   /* RS=0 */
    _delay_loop_1(T);
    PORTB &= ~E;
}

void
lcd_data(uint8_t n)
{
    lcd_wait_busy();

    _delay_loop_1(T);
    PORTB = ((n & 0xf0) >> 2) | RS | E;   /* RS=1 */
    _delay_loop_1(T);
    PORTB &= ~E;
    
    _delay_loop_1(T);
    PORTB = ((n & 0x0f) << 2) | RS | E;   /* RS=1 */
    _delay_loop_1(T);
    PORTB &= ~E;
}

int
lcdwrite(char c, FILE *f) {
    lcd_data(c);
    return 0;
}

static void
e_pulse(uint8_t c) {
    PORTB = c | E;
    _delay_loop_1(T);
    PORTB &= ~E;
    _delay_loop_1(T);
}

void
lcd_init(void)
{
    PORTB = 0;
    DDRB = 0x3c | RS | E; // enable used PORT bits as output
    PORTD &= ~RW; // PD7 is low
    DDRD |= RW; // enable PD7 as output

    _delay_ms(15);
    e_pulse((((HD44780_CMD_FUNCTION_SET | HD44780_8_BIT) >> 4) << 2)); /* RS=0=instrmode */
    _delay_ms(5); // > 4.1 ms
    e_pulse((((HD44780_CMD_FUNCTION_SET | HD44780_8_BIT) >> 4) << 2)); /* RS=0=instrmode */
    _delay_us(150); // > 100 us
    e_pulse((((HD44780_CMD_FUNCTION_SET | HD44780_8_BIT) >> 4) << 2)); /* RS=0=instrmode */
    _delay_us(150); // not specified
    e_pulse(((HD44780_CMD_FUNCTION_SET >> 4) << 2)); /* RS=0=instrmode */
    _delay_us(150); // not specified

    lcd_cmd(HD44780_CMD_FUNCTION_SET | HD44780_2_LINES);

    // let printf just write to the lcd :)
    stdout = fdevopen(lcdwrite, NULL);
}

