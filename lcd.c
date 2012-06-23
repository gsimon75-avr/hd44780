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

    PORTB &= 0xfd; // RS = 0 
    DDRB &= 0xc3; // B[2..5] are inputs
    PORTD |= 0x80; // R/#W = 1

    do {
        _delay_loop_1(T);
        PORTB |= 0x01;   /* E=0->1 */
        _delay_loop_1(T);
        lcd_status = (PINB << 2) & 0xf0;
        PORTB &= 0xfe;   /* E=1->0 */

        _delay_loop_1(T);
        PORTB |= 0x01;   /* E=0->1 */
        _delay_loop_1(T);
        PORTB &= 0xfe;   /* E=1->0 */
    } while (lcd_status & 0x80);

    PORTD &= 0x7f; // R/#W = 0
    DDRB |= 0x3c; // B[2..5] are outputs
}

void
lcd_cmd(uint8_t n)
{
    lcd_wait_busy();

    _delay_loop_1(T);
    PORTB = ((n & 0xf0) >> 2) | 0x01;   /* RS=0, E=0->1 */
    _delay_loop_1(T);
    PORTB &= 0xfe;      /* E=1->0 */
    
    _delay_loop_1(T);
    PORTB = ((n & 0x0f) << 2) | 0x01;   /* RS=0, E=0->1 */
    _delay_loop_1(T);
    PORTB &= 0xfe;      /* E=1->0 */
}

void
lcd_data(uint8_t n)
{
    lcd_wait_busy();

    _delay_loop_1(T);
    PORTB = ((n & 0xf0) >> 2) | 0x03;   /* RS=1, E=0->1 */
    _delay_loop_1(T);
    PORTB &= 0xfe;      /* E=1->0 */
    
    _delay_loop_1(T);
    PORTB = ((n & 0x0f) << 2) | 0x03;   /* RS=1, E=0->1 */
    _delay_loop_1(T);
    PORTB &= 0xfe;      /* E=1->0 */
}

int
lcdwrite(char c, FILE *f) {
    lcd_data(c);
    return 0;
}

static void
e_pulse(uint8_t c) {
    PORTB = c | 0x01;   /* E=0->1 */
    _delay_loop_1(T);
    PORTB &= 0xfe;      /* E=1->0 */
    _delay_loop_1(T);
}

void
lcd_init(void)
{
    PORTB = 0xc0;
    DDRB = 0x3f; // enable used PORT bits as output
    PORTD &= 0x7f; // PD7 is low
    DDRD |= 0x80; // enable PD7 as output

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

