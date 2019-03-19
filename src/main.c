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

// Pin macros
#define KB_PIN 1
#define BTTN1_PIN 6
#define BTTN2_PIN 7

// Default timer CMP register value
#define TMR_CMP 1

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

static struct RLstat RL_1 = {
    .moving = 1,
    .speed = 5,
    .direction = 0,
    .position = 0
};

static struct RLstat RL_2 = {
    .moving = 1,
    .speed = 5,
    .direction = 0,
    .position = 1
};

// ISR functions
void timer0_ISR(void) __attribute__ ((interrupt ("IRQ")));
void timer1_ISR(void) __attribute__ ((interrupt ("IRQ")));
void timer2_ISR(void) __attribute__ ((interrupt ("IRQ")));
void button_ISR(void) __attribute__ ((interrupt ("IRQ")));
void keyboard_ISR(void) __attribute__ ((interrupt ("IRQ")));

// XXX: To update mode and prescaler, do we need to restart?
// Not really, but if we do so, the changes will be visible as soon as we update,
// and not only on the next timer cycle.

// Setup timer `t` with the given values
int setup_timer(enum tmr_timer t,
                int prescaler_value,
                enum tmr_div div_value,
                int timer_count,
                enum tmr_mode mode) {


    int prescaler_p;

    // Divider is the same as the timer
    int divider_p = (int) t;

    // Set timer mode
    if (tmr_set_mode(t, mode) != 0) {
        return -1;
    }

    // Set count and cmp registers
    if (tmr_set_count(t, timer_count, TMR_CMP) != 0) {
        return -1;
    }

    // Set divider value
    if (tmr_set_divider(divider_p, div_value) != 0) {
        return -1;
    }

    // Which prescaler should we use?
    // Prescaler are shared every two timers
    if (t == TIMER0 || t == TIMER1) {
        prescaler_p = 0;
    } else if (t == TIMER2 || t == TIMER3) {
        prescaler_p = 1;
    } else if (t == TIMER4 || t == TIMER5) {
        prescaler_p = 2;
    } else {
        return -1;
    }

    if (tmr_set_prescaler(prescaler_p, prescaler_value) != 0) {
        return -1;
    }

    // Flush the buffer registers and force-update the timer
    return tmr_update(t);
}

// Timer ISR, implements drawing logic
void timer0_ISR(void) {
    // If it's not moving, no need to redraw
    if (!RL_1.moving) {
        goto cleanup_timer_0_isr;
    }

    if (RL_1.direction) {
        RL_1.position = modulo(RL_1.position + 1, 6);
    } else {
        RL_1.position = modulo(RL_1.position - 1, 6);
    }

    // Redraw
    D8Led_2segments(RL_1.position, RL_2.position);

cleanup_timer_0_isr:
    // Need to clear the pending flag
    ic_cleanflag(INT_TIMER0);
}

void timer1_ISR(void) {
    // If it's not moving, no need to redraw
    if (!RL_2.moving) {
        goto cleanup_timer_1_isr;
    }

    if (RL_2.direction) {
        RL_2.position = modulo(RL_2.position + 1, 6);
    } else {
        RL_2.position = modulo(RL_2.position - 1, 6);
    }

    // Redraw
    D8Led_2segments(RL_1.position, RL_2.position);

cleanup_timer_1_isr:
    // Need to clear the pending flag
    ic_cleanflag(INT_TIMER1);
}

void timer2_ISR(void) {
    leds_switch();
    ic_cleanflag(INT_TIMER2);
}

void button_ISR(void) {
    // EXTINTPND is a 4-bit register, telling which lines are active
    // This ISR is shared by lines 4, 5, 6 and 7
    unsigned int which_eint_line = rEXTINTPND;

    // Get the two MS bits
    unsigned int buttons = (which_eint_line >> 2) & 0x3;

    // Push button 1 was pressed
    if (buttons & BUT1) {
        RL_1.moving = !RL_1.moving;
    }

    if (buttons & BUT2) {
        RL_2.moving = !RL_2.moving;
    }

    // Wait for debounce
    Delay(2000);

    // Clean ISR flags

    // Write back the default value
    rEXTINTPND = which_eint_line;

    // Now clear the flag on the interrupt controller
    ic_cleanflag(INT_EINT4567);
}

void keyboard_ISR(void) {
    int key;
    enum digital key_state = LOW;

    // Wait for debounce
    Delay(200);

    key = kb_scan();
    if (key == -1) {
        goto exit_kb_isr;
    }

    D8Led_digit(key);

    switch (key) {
        case 0:
            // Set timer 0: freq 2s, presc 255, count 62500, div 1/8
            setup_timer(TIMER0, 255, D1_8, 62500, RELOAD);
            break;
        case 1:
            // Set timer 0: freq 1s, presc 255, count 31250, div 1/8
            setup_timer(TIMER0, 255, D1_8, 31250, RELOAD);
            break;
        case 2:
            // Set timer 0: freq 0.5s, presc 255, count 15625, div 1/8
            setup_timer(TIMER0, 255, D1_8, 15625, RELOAD);
            break;
        case 3:
            // Set timer 0: freq 0.25s, presc 255, count 15625, div 1/4
            setup_timer(TIMER0, 255, D1_4, 15625, RELOAD);
            break;
        case 4:
            // Set timer 1: freq 2s, presc 255, count 62500, div 1/8
            setup_timer(TIMER1, 255, D1_8, 62500, RELOAD);
            break;
        case 5:
            // Set timer 1: freq 1s, presc 255, count 31250, div 1/8
            setup_timer(TIMER1, 255, D1_8, 31250, RELOAD);
            break;
        case 6:
            // Set timer 1: freq 0.5s, presc 255, count 15625, div 1/8
            setup_timer(TIMER1, 255, D1_8, 15625, RELOAD);
            break;
        case 7:
            // Set timer 1: freq 0.25s, presc 255, count 15625, div 1/4
            setup_timer(TIMER1, 255, D1_4, 15625, RELOAD);
            break;
        case 8:
            leds_display(0x3);
            // Set timer 2: freq 2s, presc 255, count 62500, div 1/8
            setup_timer(TIMER2, 255, D1_8, 62500, ONE_SHOT);
            if (tmr_isrunning(TIMER2)) {
                tmr_stop(TIMER2);
            }
            tmr_start(TIMER2);
            break;
        case 9:
            leds_display(0x3);
            // Set timer 2: freq 1s, presc 255, count 31250, div 1/8
            setup_timer(TIMER2, 255, D1_8, 31250, ONE_SHOT);
            if (tmr_isrunning(TIMER2)) {
                tmr_stop(TIMER2);
            }
            tmr_start(TIMER2);
            break;
        case 10:
            leds_display(0x3);
            setup_timer(TIMER2, 255, D1_8, 15625, RELOAD);
            if (!tmr_isrunning(TIMER2)) {
                tmr_start(TIMER2);
            }
            break;
        case 11:
            leds_display(0x0);
            // Stops timer 2 if running
            if (tmr_isrunning(TIMER2)) {
                tmr_stop(TIMER2)
            }
            break;
        default:
            break;
    }

    // Wait until key is depressed
    // When LOW -> key is pressed, interrupt is being generated
    // When HIGH -> key is depressed (interrupt is cleared)
    do {
        // Docs say that reading from a functional pin is undefined,
        // but it works in our case.
        portG_read(KB_PIN, &key_state);
    } while(key_state == LOW);

exit_kb_isr:
    // Wait for debounce
    Delay(200);

    // Clear pending interrupts on line EINT1
    ic_cleanflag(INT_EINT1);
}

int setup(void) {
    // Initialize LEDs
    leds_init();

    // Initialize and light up appropriate segment
    D8Led_init();
    D8Led_2segments(RL_1.position, RL_2.position);

    // ------------------------------------------------------------
    // Port G configuration
    // ------------------------------------------------------------

    // Enable button handling via interrupts (pins 6 and 7)
    // * Enable pin mode to EINT
    // * Set up pull-up registry
    // * Set up interrupt mode for that pin
    portG_conf(BTTN1_PIN, EINT);
    portG_conf_pup(BTTN1_PIN, ENABLE);
    portG_eint_trig(BTTN1_PIN, FALLING);

    portG_conf(BTTN2_PIN, EINT);
    portG_conf_pup(BTTN2_PIN, ENABLE);
    portG_eint_trig(BTTN2_PIN, FALLING);

    // Enable keyboard handling via interrupts (pin 1 on port G)
    // Pull-up is not really needed for keyboard, but we might as well
    portG_conf(KB_PIN, EINT);
    portG_conf_pup(KB_PIN, ENABLE);
    portG_eint_trig(KB_PIN, FALLING);

    // ------------------------------------------------------------
    // Timer 0 configuration
    // ------------------------------------------------------------

    // Set up timer 0 to fire every 2s
    // We need a prescaler value of 255, and a divider of 8
    setup_timer(TIMER0, 255, D1_8, 62500, RELOAD);
    setup_timer(TIMER1, 255, D1_8, 62500, RELOAD);
    if (RL_1.moving) {
        tmr_start(TIMER0);
    }

    if (RL_2.moving) {
        tmr_start(TIMER1);
    }

    // ------------------------------------------------------------
    // Set up ISR handlers
    // ------------------------------------------------------------

    // Register ISR for keyboard (EINT1) and timer (TIMER0)
    pISR_EINT1 = (int) keyboard_ISR;
    pISR_TIMER0 = (int) timer0_ISR;
    pISR_TIMER1 = (int) timer1_ISR;
    pISR_TIMER2 = (int) timer2_ISR;

    // Register ISR for buttons (shared ISR for lines 4, 5, 6 and 7)
    // Will need to determine if buttons were pressed inside button_ISR
    // This is done reading the EXTINTPND[3:0] register
    pISR_EINT4567 = (int) button_ISR;

    // ------------------------------------------------------------
    // Configure interrupt controller
    // ------------------------------------------------------------

    // Reset Interrupt Controller to default configuration
    ic_init();

    // Keep FIQ disabled and enable IRQ in vectorized mode
    ic_conf_fiq(DISABLE);
    ic_conf_irq(ENABLE, VEC);

    // Configure timer, push buttons keyboard lines in IRQ
    ic_conf_line(INT_TIMER0, IRQ);
    ic_conf_line(INT_TIMER1, IRQ);
    ic_conf_line(INT_TIMER2, IRQ);
    ic_conf_line(INT_EINT4567, IRQ); // Buttons
    ic_conf_line(INT_EINT1, IRQ); // Keyboard

    // Enable timer, push buttons and keyboard lines
    ic_enable(INT_TIMER0);
    ic_enable(INT_TIMER1);
    ic_enable(INT_TIMER2);
    ic_enable(INT_EINT4567);
    ic_enable(INT_EINT1);

    // Finally, unmask the global register
    // If disabled, no interrupt will be serviced, even
    // if the individual line is enabled
    ic_enable(INT_GLOBAL);

    // ------------------------------------------------------------
    // Calibrate timer
    // ------------------------------------------------------------

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
