#include <stdio.h>
#include "44b.h"
#include "button.h"
#include "leds.h"
#include "utils.h"
#include "D8Led.h"
#include "gpio.h"

#define PIN_BUT1 6
#define PIN_BUT2 7

struct RLstat {
    // 0 -> still, 1 -> moving
    int moving;
    // speed in loops
    int speed_idx;
    int speed;
    // how many iterations in the last loop
    // if 0, will move in `direction`
    // when moved, should reset to `speed`
    int iter;
    // 0 -> counterclockwise, 1 -> clockwise
    int direction1;
    int direction2;
    // position represented as index in
    // Segments[] (see D8Led.c)
    // Should be always between 0 and 5
    // (modulo movement, always in circles)
    int position1;
    int position2;
};

static struct RLstat RL = {
    .moving = 1,
    .speed_idx = 0,
    .speed = 3,
    .iter = 1,
    .direction1 = 0,
    .direction2 = 0,
    .position1 = 0,
    .position2 = 1,
};

// Keep track of button 2 press events
static unsigned int button2_counter = 0;
static unsigned int Speeds[] = {3, 6, 9, 12};

// Modulo for both positive and negative numbers
int modulo(int x, int n) {
    return ((x % n) + n) % n;
}

// Will set up all HW registers
int setup(void) {
    // Initialize LEDs
    leds_init();

    // Initialize and light up appropriate segment
    D8Led_init();
    D8Led_2segments(RL.position1, RL.position2);

    // Init buttons manually, and enable pull-up registry
    portG_conf(PIN_BUT1, INPUT);
    portG_conf_pup(PIN_BUT1, ENABLE);

    portG_conf(PIN_BUT2, INPUT);
    portG_conf_pup(PIN_BUT2, ENABLE);

    // Calibrate timer
    Delay(0);
    return 0;
}

// Main body loop
int loop(void) {
    // Check if any pushbuttons were pressed
    unsigned int buttons = read_button();

    // Push button 1 was pressed
    if (buttons & BUT1) {
        // First, turn off both LEDs
        led1_off();
        led2_off();

        // Increase speed
        RL.speed_idx = modulo(RL.speed_idx + 1, 4);
        RL.speed = Speeds[RL.speed_idx];
    }

    // Push button 2 was pressed
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

        // Toggle direction1
        RL.direction1 = !RL.direction1;
    }

    // If RL is moving
    if (RL.moving) {
        RL.iter--;
        // If reached 0, advance RL.position in RL.direction
        // and redraw the segment
        if (RL.iter == 0) {
            // Advance
            if (RL.direction1) {
                RL.position1 = modulo(RL.position1 + 1, 6);
            } else {
                RL.position1 = modulo(RL.position1 - 1, 6);
            }

            if (RL.direction2) {
                RL.position2 = modulo(RL.position2 + 1, 6);
            } else {
                RL.position2 = modulo(RL.position2 - 1, 6);
            }

            // Reset iter field
            RL.iter = RL.speed;

            // Redraw
            D8Led_2segments(RL.position1, RL.position2);
        }
    }

    // Wait for 200ms, so this loop executes 5 times/second
    Delay(2000);
    return 0;
}


int main(void) {
    setup();
    while (1) {
        loop();
    }
}
