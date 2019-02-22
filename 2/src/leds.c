#include "44b.h"
#include "leds.h"
#include "gpio.h"

// Bitmasks
#define LED1 0x01
#define LED2 0x02

// LED pins
#define PIN_LED1 9
#define PIN_LED2 10

// LEDs are connected to pins 9 and 10 on port B
// so we should conf Port B as output mode
// and write '1' when we want to turn them on (rPDATB)

// LED status. Only look at the two lowest bits,
// each for a single LED
// 0 -> OFF, 1 -> ON
static unsigned int status = 0;

void leds_init(void) {
    // Should set up pins 9 and 10 as output
    portB_conf(PIN_LED1, OUTPUT);
    portB_conf(PIN_LED2, OUTPUT);

    leds_display(status);
}

void led1_on(void) {
    status |= LED1;
    leds_display(status);
}

void led1_off(void) {
    status &= ~LED1;
    leds_display(status);
}

void led2_on(void) {
    status |= LED2;
    leds_display(status);
}

void led2_off(void) {
    status &= ~LED2;
    leds_display(status);
}

void led1_switch(void) {
    status ^= LED1;
    leds_display(status);
}

void led2_switch(void) {
    status ^= LED2;
    leds_display(status);
}

void leds_switch(void){
    status ^= (LED1 | LED2);
    leds_display(status);
}

// Turn on an LED
//
// leds_status:
// (00) -> Both off
// (01) -> LED 2 off, LED 1 on
// (10) -> LED 2 on, LED 1 off
// (11) -> Both on
void leds_display(unsigned int leds_status) {
    status = leds_status;

    // LED 1 is set, turn on (write 0 on port b)
    if(status & LED1) {
        portB_write(PIN_LED1, LOW);
    } else {
        portB_write(PIN_LED1, HIGH);
    }

    // LED 2 is set, turn on (write 0 on port b)
    if(status & LED2) {
        portB_write(PIN_LED2, LOW);
    } else {
        portB_write(PIN_LED2, HIGH);
    }
}
