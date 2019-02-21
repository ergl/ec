#include "44b.h"
#include "leds.h"
#include "gpio.h"

// Bitmasks
#define LED1 0x01
#define LED2 0x02
#define BIT_LED1 9
#define BIT_LED2 10

// LED status. Only look at the two lowest bits,
// each for a single LED
// 0 -> OFF, 1 -> ON
static unsigned int status = 0;

void leds_init(void) {
    // TODO(borja): Should set up pins 9 and 10 on Port B
    // Need to set up both as output pins, using gpio.h API

    // TODO(borja): Implement
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
// LED1 -> Turn on LED 1 (?)
// LED2 -> Turn on LED 2 (?)
void leds_display(unsigned int leds_status) {
    status = leds_status;

    // LED 1
    if(status & LED1) {
        // Using gpio.h API, set LED 1 bit on Port B to LOW
    } else {
        // Using gpio.h API, set LED 1 bit on Port B to HIGH
    }

    // LED 2
    if(status & LED2) {
        // Using gpio.h API, set LED 2 bit on Port B to LOW
    } else {
        // Using gpio.h API, set LED 2 bit on Port B to HIGH
    }
}
