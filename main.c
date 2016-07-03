

#include <avr/io.h>
#include <avr/sleep.h>
#include <util/delay.h>

#include <avr/wdt.h>

#include <stdio.h>
int uart_putchar(char c, FILE *stream);
int uart_getchar(FILE *stream);
FILE uart_str = FDEV_SETUP_STREAM(uart_putchar, uart_getchar, _FDEV_SETUP_RW);

/* ------------------------------------------------------------------------- */
#define UCSRA UCSR0A
#define UCSRB UCSR0B
#define UCSRC UCSR0C
#define UBRRH UBRR0H
#define UBRRL UBRR0L
#define UDRE UDRE0
#define UDR UDR0
#define RXC RXC0
#define RXEN RXEN0
#define TXEN TXEN0
#define UCSZ1 UCSZ01
#define UCSZ0 UCSZ00
#define U2X U2X0
void config_baud115200() {
  // U2X=1
  UCSRA |= (1 << U2X);
#undef BAUD // avoid compiler warning
#define BAUD 115200
#include <util/setbaud.h>
  UBRRH = UBRRH_VALUE;
  UBRRL = UBRRL_VALUE;
  UCSRB = _BV(TXEN) | _BV(RXEN); /* tx/rx enable */
  UCSRC = (1 << UCSZ1) | (1 << UCSZ0);
}
void config_baud9600() {
  // U2X=1
  UCSRA |= (1 << U2X);
#undef BAUD // avoid compiler warning
#define BAUD 9600
#include <util/setbaud.h>
  UBRRH = UBRRH_VALUE;
  UBRRL = UBRRL_VALUE;
  UCSRB = _BV(TXEN) | _BV(RXEN); /* tx/rx enable */
  UCSRC = (1 << UCSZ1) | (1 << UCSZ0);
}

/*
 * Bits that are set inside interrupt routines, and watched outside in
 * the program's main loop.
 */
volatile struct {
  uint8_t tmr_int : 1;
  uint8_t adc_int : 1;
  uint8_t rx_int : 1;
} intflags;

uint16_t adcval;

void putChar(char c) {

  loop_until_bit_is_set(UCSRA, UDRE);
  UDR = c;
}
int uart_putchar(char c, FILE *stream);
int uart_putchar(char c, FILE *stream) { putChar(c); }
int uart_getchar(FILE *stream) { return 0; }

unsigned char hasChar() { return (UCSR0A & (1 << RXC0)); }
unsigned char getChar(void) {
  /* Wait for data to be received */
  while (!(UCSR0A & (1 << RXC0)))
    ;
  /* Get and return received data from buffer */
  return UDR;
}

#define MAIN_LED 2
#define BLUE_LED 5
#define RED_LED 7
#define GREEN_LED 6

void ledOff(uint8_t led) { PORTD |= 1 << led; }
void ledOn(uint8_t led) { PORTD &= ~(1 << led); }

void mainOn() { ledOff(MAIN_LED); }
void mainOff() { ledOn(MAIN_LED); }

uint8_t isPressed() {
  if (PIND & (1 << 3))
    return 0;
  else
    return 1;
}

void buzzer(uint8_t on) {
  if (on) {
    ledOn(BLUE_LED);
    mainOn();
    //      PORTB|=1<<1;
  } else {
    ledOff(BLUE_LED);
    mainOff();
    PORTB &= ~(1 << 1);
  }
}

uint8_t grace __attribute__((section(".noinit")));
uint8_t notice __attribute__((section(".noinit")));
int main() {
  wdt_enable(WDTO_8S);
  wdt_reset();
  DDRD = (1 << 2) | (1 << 5) | (1 << 6) | (1 << 7);
  DDRB = 1 << 1;
  PORTD = (1 << 3) | (1 << 5) | (1 << 6) | (1 << 7);
  DDRC = 1 << 4;
  PORTC = (1 << 4);
  config_baud9600();
  stdout = stdin = &uart_str;
  mainOff();
  ADMUX = _BV(ADLAR) | 5;
  DIDR0 = _BV(5);
  ADCSRA = _BV(ADEN) | _BV(ADIE) | _BV(ADATE) | _BV(ADPS1);
  ADCSRA |= _BV(ADSC);
  buzzer(0);
  while (1) {
    // printf("START %d\n\r", grace);
    mainOff();
    _delay_ms(20);
    uint8_t start = ADCH;
    mainOn();
    _delay_ms(20);
    uint8_t stop = ADCH;
    mainOff();

    uint8_t diff;
    if (start < stop)
      diff = 0;
    else
      diff = start - stop;

    // printf("DONE %d\n\r",diff);
    if (diff > 80) {
      PORTC = 0;
      PORTB = 0;
      ledOff(RED_LED);
      ledOff(BLUE_LED);
      if (notice < 3) {
        notice++;
        ledOn(GREEN_LED);
      }
      grace = 0;
      sleep_mode();
      while (1)
        ;
    } else {
      notice = 0;
      if (grace < 37) {
        ledOn(BLUE_LED);
        grace++;
        sleep_mode();
        while (1)
          ;
      } else {
        ledOff(BLUE_LED);
        ledOn(RED_LED);
        buzzer(1);
        if (isPressed()) {
          grace = 0;
        }
      }
    }
    wdt_reset();
  }

}
