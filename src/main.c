#include <stdio.h>
#include "44b.h"
#include "button.h"
#include "leds.h"
#include "utils.h"
#include "D8Led.h"
#include "intcontroller.h"
#include "timer.h"
#include "gpio.h"
#include "keyboard.h"

#define PIN_BUT1 6
#define PIN_BUT2 7

struct RLstat {
    // 0 -> still, 1 -> moving
    int moving;
    // speed in loops
    int speed;
    // 0 -> counterclockwise, 1 -> clockwise
    int direction;
    // position represented as index in
    // Segments[] (see D8Led.c)
    // Should be always between 0 and 5
    // (modulo movement, always in circles)
    int position;
};

static struct RLstat RL = {
    .moving = 0,
    .speed = 5,
    .direction = 0,
    .position = 0,
};

// Keep track of button 2 press events
static unsigned int button2_counter = 0;

// ISR functions
void timer_ISR(void) __attribute__ ((interrupt ("IRQ")));
void button_ISR(void) __attribute__ ((interrupt ("IRQ")));
void keyboard_ISR(void) __attribute__ ((interrupt ("IRQ")));

// Timer ISR, implements drawing logic
void timer_ISR(void) {

    // If it's not moving, no need to redraw
    if (!RL.moving) {
        return;
    }

    if (RL.direction) {
        RL.position = modulo(RL.position + 1, 6);
    } else {
        RL.position = modulo(RL.position - 1, 6);
    }

    // Redraw
    D8Led_segment(RL.position);
}

void button_ISR(void) {
    unsigned int whicheint = rEXTINTPND;
    unsigned int buttons = (whicheint >> 2) & 0x3;

    // FIXME: Need to check which button was pressed?
    // unsigned int buttons = read_button();

    // Push button 1 was pressed
    if (buttons & BUT1) {
        // First, turn off both LEDs
        led1_off();
        led2_off();

        // Toggle RL direction
        RL.direction = !RL.direction;
    }

    if (buttons & BUT2) {
        // First, increment internal counter, and toggle led accordingly
        // FIXME(borja): Might overflow? Swap to a boolean toggle
        button2_counter++;
        if (button2_counter & 1) {
            // Odd, toggle LED 2
            led2_switch();
        } else {
            // Even, toggle LED 1
            led1_switch();
        }

        // Then, toggle RL.moving
        RL.moving = !RL.moving;
    }

    // Wait for debounce
    Delay(2000);

    // Clean extintpnd flag
    // TODO: We must clear the pending interrupt flag in rEXTINTPND
    // Write 1 on the flags we want to clear (the ones corresponding to the
    // buttons that were pushed)

    // TODO
    // rEXTINTPND = ...

    // TODO: Clear interrupt flag on the interrupt controller ?
}

void keyboard_ISR(void) {
    int key;

    // Wait for debounce
    Delay(200);

    // Get key (TODO: Implement)
    key = kb_scan();

    if (key == -1) {
        goto exit_kb_isr;
    }

    // TODO: Display key on the 8-segment display (using API on D8Led)

    switch (key) {
        case 0:
            // TODO: Set timer 0: freq 2s, count 62500, div 1/8
            break;
        case 1:
            // TODO: Set timer 0: freq 1s, count 31250, div 1/8
            break;
        case 2:
            // TODO: Set timer 0: freq 0.5s, count 15625, div 1/8
            break;
        case 3:
            // TODO: Set timer 0: freq 0.25s, count 15625, div 1/4
            break;
        default:
            break;
    }

    // TODO: Wait until key is depressed
    // See outline, page 5
    while ( /* TODO: True when key is depressed */ ) {
        // Noop wait
    }

    /* Eliminar rebotes de depresión */

exit_kb_isr:
    // Wait for debounce
    Delay(200);

    // TODO: Clear pending interrupts on line EINT1 (rI_ISPC register)
}

// Will set up all HW registers
int setup(void) {
    // Initialize LEDs
    leds_init();

    // Initialize and light up appropriate segment
    D8Led_init();
    D8Led_segment(RL.position);

    // Init buttons manually, and enable pull-up registry
    // TODO: Configure Port G to generate external interrupts.
    // Do this for leds, kb and buttons
    // For pins 1, 6 and 7 (G port), enable interrupts on falling edge
    // and enable appropriate pull-up registers

    // TODO: Set up timer

    if (RL.moving) {
        tmr_start(TIMER0);
    }

    // TODO: Register ISR
    // pISR_TIMER0 = ...
    // pISR_EINT4567 = ...
    // pISR_EINT1 = ...

    ic_init();

    // TODO: Set up interrupt lines (using intcontroller.h API)
    // 1. Enable IRQ line in vectorized mode
    // 2. Disable FIQ line
    // 3. Set up INT_TIMER0 in IRQ mode
    // 4. Set up INT_EINT4567 in IRQ mode
    // 5. Set up INT_EINT1 in IRQ mode
    // 6. Enable INT_TIMER0
    // 7. Enable INT_EINT4567
    // 8. Enable INT_EINT1


    // Calibrate timer
    Delay(0);
    return 0;
}

int loop(void) {
    return 0;
}

int main(void) {
    setup();
    while (1) {
        loop();
    }
}
