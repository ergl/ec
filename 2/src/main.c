#include <stdio.h>
#include "44b.h"
#include "button.h"
#include "leds.h"
#include "utils.h"
#include "D8Led.h"
#include "gpio.h"

struct RLstat {
    // 0 -> still, 1 -> moving
    int moving;
    // speed in loops
    int speed;
    // how many iterations in the last loop
    // if 0, will move in `direction`
    // when moved, should reset to `speed`
    int iter;
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
    .iter = 0,
    .direction = 0,
    .position = 0,
};

// Will set up all HW registers
int setup(void) {
    // TODO(borja): Implement
    // Initialize LEDs
    leds_init();

    // Initialize and light up appropriate segment
    D8Led_init();
    D8Led_segment(RL.position);

    // TODO(borja): Set up push buttons on port G
    // Manually using gpio.h API
    // Set up pins 6 and 7 on Port G as input
    // Also remember to set up the pull-up reg for both

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
        // TODO(borja): Implement button 1 behaviour
        // Using leds.h API, toggle LED 1 status
        // Or turn off both LEDS (?) (comments vs outline)
        // Toggle 8 led direction (RL.direction)
    }

    // Push button 2 was pressed
    if (buttons & BUT2) {
        // TODO(borja): Implement button 2 behaviour

        // Using leds.h API, toggle LED 2 status (?) (comments vs outline)
        // Or toggle LED status according to internal counter
        // If counter is even -> toggle LED 1
        // If odd -> toggle LED 2

        // Toggle RL.moving state
        // If putting it in movement (0 -> 1),
        // refresh RL.iter to RL.speed
    }

    // If RL is moving
    if (RL.moving) {
        RL.iter--;
        // When reaching 0, we should advance
        // RL.position 1 unit in RL.direction
        // Rememeber: RL.position should always be in 0-5 range
        if (RL.iter == 0) {
            // TODO(borja): Advance logic
            // Using D8LEd.h API
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
