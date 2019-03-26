#include <stdio.h>
#include "44b.h"
#include "leds.h"
#include "utils.h"
#include "D8Led.h"
#include "intcontroller.h"
#include "timer.h"
#include "gpio.h"
#include "keyboard.h"

// Pin macros
#define KB_PIN 1
#define BUF_SIZE 4

// Timer helper methods
enum tmr_seconds {
    TWO_SECS = 0,
    ONE_SEC = 1,
    HALF_SEC = 2,
    QUARTER_SEC = 3
};

// Default timer CMP register value
#define TMR_CMP 1

// FSM state
enum state {
    INIT = 0,
    SHOW_PASS = 1,
    GUESS = 2,
    SHOW_GUESS = 3,
    GAME_OVER = 4
};

// Global game state
enum state game_state;

static char password_buf[BUF_SIZE];
static char guess_buf[BUF_SIZE];

// ISR functions
void timer_ISR(void) __attribute__ ((interrupt ("IRQ")));
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

int set_timer_to(enum tmr_timer t, enum tmr_seconds seconds, enum tmr_mode mode) {
    int timer_count;
    int prescaler_value = 255;
    enum tmr_div div_value;

    // Valid second definition?
    if (seconds < 0 || seconds > 3) {
        return -1;
    }

    if (seconds == QUARTER_SEC) {
        div_value = D1_4;
    } else {
        div_value = D1_8;
    }

    switch (seconds) {
        case TWO_SECS:
            timer_count = 62500;
            break;
        case ONE_SEC:
            timer_count = 31250;
            break;
        case HALF_SEC: // Fall-through
        case QUARTER_SEC:
            timer_count = 15625;
            break;
        default:
            return -1;
    }

    return setup_timer(t, prescaler_value, div_value, timer_count, mode);
}

// TODO(borja): Timer 0 ISR logic
void timer_ISR(void) {
    // Need to clear the pending flag
    ic_cleanflag(INT_TIMER0);
}

// TODO(borja): Implement keyboard ISR
void keyboard_ISR(void) {
    int key;
    enum digital key_state = LOW;

    // Wait for debounce
    Delay(200);

    key = kb_scan();
    if (key == -1) {
        goto exit_kb_isr;
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
    // Initialize 8-segment display
    D8Led_init();

    // ------------------------------------------------------------
    // Port G configuration
    // ------------------------------------------------------------

    // Enable keyboard handling via interrupts (pin 1 on port G)
    // Pull-up is not really needed for keyboard, but we might as well
    portG_conf(KB_PIN, EINT);
    portG_conf_pup(KB_PIN, ENABLE);
    portG_eint_trig(KB_PIN, FALLING);

    // ------------------------------------------------------------
    // Timer 0 configuration
    // ------------------------------------------------------------
    set_timer_to(TIMER0, ONE_SEC, RELOAD);

    // ------------------------------------------------------------
    // Set up ISR handlers
    // ------------------------------------------------------------

    // Register ISR for keyboard (EINT1) and timer (TIMER0)
    pISR_EINT1 = (int) keyboard_ISR;
    pISR_TIMER0 = (int) timer_ISR;


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
    ic_conf_line(INT_EINT1, IRQ); // Keyboard

    // Enable timer, push buttons and keyboard lines
    ic_enable(INT_TIMER0);
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
    switch (game_state) {
        case INIT:
            // TODO(borja) Implement
            // D8 = C
            // Fill the buffer while key != F
            // Check that buffer size >= 4, otherwise error
            // FIXME(borja): if (OK) SHOW_PASS else INIT
            game_state = SHOW_PASS;
            break;
        case SHOW_PASS:
            // TODO(borja): Implement
            // Every second, show pass on D8
            game_state = GUESS;
            break;
        case GUESS:
            // TODO(borja): Implement
            // D8 = F
            // Fill the buffer while key != F
            // Check that buffer size >= 4, otherwise error
            // FIXME(borja): if (OK) SHOW_GUESS else GUESS
            game_state = SHOW_GUESS;
            break;
        case SHOW_GUESS:
            // TODO(borja): Implement
            // Every second, show guess on D8
            game_state = GAME_OVER;
            break;
        case GAME_OVER:
            // TODO(borja): Check guess == pass
            // FIXME(borja): if (OK) INIT else GUESS
            game_state = INIT;
            break;
    }

    return 0;
}

int main(void) {
    setup();
    while (1) {
        loop();
    }
}
