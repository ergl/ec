#include <stdio.h>
#include "44b.h"
#include "leds.h"
#include "utils.h"
#include "D8Led.h"
#include "intcontroller.h"
#include "timer.h"
#include "gpio.h"
#include "keyboard.h"
#include "ring.h"

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

// Password, guess and intermediate buffers
static char password_buf[BUF_SIZE];
static char guess_buf[BUF_SIZE];
static char backing_buffer[BUF_SIZE];

// Ring buffer holding the data from user
static struct ring_t ring_buffer;

// Input done is set 1 to from ISR when user is done
// pressing keys
volatile static int input_done = 0;

// Show done is set to 1 from ISR when we're done showing
// the user input
volatile static int show_done = 0;

// How many characters do we read from ring buffer?
volatile static int show_watermark = 0;

// Points to the target buffer where the ring should copy its contents
// Will target either password or guess, depending on the use-case.
volatile static char* target_buffer = NULL;

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

// This ISR will keep printing data from the ring buffer
// and moving it into the pointer denoted by target_buffer
// It will also print the value on the 8-segment display
void timer_ISR(void) {
    char data;
    // Keep track of index to print
    static int current_pos = 0;

    // If done, stop the timer, and signal end of show
    if (current_pos == show_watermark) {
        tmr_stop(TIMER0);
        current_pos = 0;
        show_done = 1;
    } else {
        // Else, pop a value from the buffer, and push it into
        // the target buffer. Display it, and advance the pointer
        ring_get(&ring_buffer, &data);
        if (target_buffer != NULL) {
            target_buffer[current_pos] = data;
        }
        D8Led_digit(data);
        ++current_pos;
    }

    // Need to clear the pending flag
    ic_cleanflag(INT_TIMER0);
}

// Will store user input on a ring buffer until the user presses the 'F' key.
// At that point it will signal it by setting `input_done` to 1.
void keyboard_ISR(void) {
    int key;
    int should_disable_interrupt = 0;
    enum digital key_state = LOW;

    // Wait for debounce
    Delay(200);

    key = kb_scan();
    if (key == -1) {
        goto exit_kb_isr;
    }

    // If key is F, signal end of user input, else store on ring
    if (key == 0xF) {
        should_disable_interrupt = 1;
        input_done = 1;
    } else {
        // Will only store 4 keys, overwrite otherwise
        ring_put(&ring_buffer, key);
    }

exit_kb_isr:

    // Wait until key is depressed
    do { portG_read(KB_PIN, &key_state); } while(key_state == LOW);

    // Wait for debounce
    Delay(200);

    // If this is the last key that should be read, disable interrupts
    if (should_disable_interrupt == 1) {
        ic_disable(INT_EINT1);
    }

    // Clear pending interrupts on line EINT1
    ic_cleanflag(INT_EINT1);
}

// Reads user input into the ring buffer
int read_user_input() {
    // Will read user input into the ring buffer
    // by repeteadly serving the keyboard ISR.

    // To do that, reset the buffer (so we don't have any
    // data from previous attempts)
    ring_reset(&ring_buffer);
    // Reset the input flag
    input_done = 0;
    // Enable the interrupts (will be disabled by the ISR itself)
    ic_enable(INT_EINT1);

    // input_done will be 1 when the ISR reads the `F` key from the user
    while(input_done == 0);

    // Return the number of keys read (size of the ring buffer)
    return ring_size(&ring_buffer);
}

// Print the contents of the buffer at 1 char/s
// and push them into `target_buffer`
void print_and_transfer() {
    // Reset show_done and show_watermark
    show_done = 0;
    show_watermark = BUF_SIZE;
    // Start the timer (will be stopped from ISR)
    tmr_start(TIMER0);
    // show_done will be 1 when the time ISR stops printing
    while (show_done == 0);
}

void print_password() {
    target_buffer = password_buf;
    print_and_transfer();
}

void print_guess() {
    target_buffer = guess_buf;
    print_and_transfer();
}

void print_result() {
    target_buffer = NULL;
    print_and_transfer();
}

int check_show_result() {
    int idx;
    int match = 1;

    for (idx = 0; idx < BUF_SIZE; ++idx) {
        if (password_buf[idx] != guess_buf[idx]) {
            match = 0;
            break;
        }
    }

    ring_reset(&ring_buffer);
    if (match == 1) {
        ring_put(&ring_buffer, 0xA);
        ring_put(&ring_buffer, 0xA);
    } else {
        ring_put(&ring_buffer, 0xE);
        ring_put(&ring_buffer, 0xE);
    }

    print_result();
    return match;
}

int setup(void) {
    // Initialize ring buffer
    ring_init(&ring_buffer, backing_buffer, BUF_SIZE);

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

    // Enable timer line (keyboard line is enabled during key input)
    ic_enable(INT_TIMER0);
    ic_disable(INT_EINT1);

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
            D8Led_digit(0xC);
            while (read_user_input() < 4) {
                D8Led_digit(0xE);
            }
            game_state = SHOW_PASS;
            break;

        case SHOW_PASS:
            print_password();
            // FIXME(borja): Need the delay? (before or after print?)
            Delay(10000);
            game_state = GUESS;
            break;

        case GUESS:
            D8Led_digit(0xF);
            while (read_user_input() < 4) {
                D8Led_digit(0xE);
            }
            game_state = SHOW_GUESS;
            break;

        case SHOW_GUESS:
            print_guess();
            // FIXME(borja): Need the delay? (before or after print?)
            Delay(10000);
            game_state = GAME_OVER;
            break;

        case GAME_OVER:
            // check_show_result result will return if game is won or not
            if (check_show_result() == 1) {
                game_state = INIT;
            } else {
                game_state = GUESS;
            }
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
