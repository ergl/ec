
#include "44b.h"
#include "timer.h"

// Set the prescaler `p` to the given `value`
// Each `p` is shared among two timers
// p = 0 for timer 0 and 1
// p = 1 for timer 2 and 3
// p = 2 for timer 4 and 5
int tmr_set_prescaler(int p, int value) {
    int offset = p*8;
    value &= 0xFF;

    if (p < 0 || p > 3) {
        return -1;
    }

    // TODO: Write `value` after `offset` position on rTCFG0
    // This will set the prescaler value for the p module

    return 0;
}

int tmr_set_divider(int d, enum tmr_div div) {
    int ret = 0;
    int pos = d * 4

    int invalid_timer = (d < 0 || d > 5);
    int invalid_divisor = (div == D1_32 && d > 3) ||
                          (div == EXTCLK && d != 5) ||
                          (div == TCLK && d != 4);

    if (invalid_timer || invalid_divisor) {
        return -1;
    }

    if (div == EXTCLK || div == TCLK) {
        div = 4;
    }

    // TODO: Write `div` after `pos` on rTCFG1
    // This will set the divider value for divisor d

    return 0;
}

int tmr_set_count(enum tmr_timer t, int count, int cmp) {
    int err = 0;
    switch (t) {
        case TIMER0:
            // TODO: Set `count` on rTCNTB0 and `cmp` on rTCMPB0
             break;
        case TIMER1:
            // TODO: Set `count` on rTCNTB1 and `cmp` on rTCMPB1
             break;
        case TIMER2:
            // TODO: Set `count` on rTCNTB2 and `cmp` on rTCMPB2
             break;
        case TIMER3:
            // TODO: Set `count` on rTCNTB3 and `cmp` on rTCMPB3
             break;
        case TIMER4:
            // TODO: Set `count` on rTCNTB4 and `cmp` on rTCMPB4
             break;
        case TIMER5:
            // TODO: Set `count` on rTCNTB5 and `cmp` on rTCMPB5
             break;
        default:
            err = -1;
    }

    return err;
}

int tmr_update(enum tmr_timer t) {
    int pos = t * 4;

    if (t < 0 || t > 5) {
        return -1;
    }

    // Skip dead zone
    if (t > 0) {
        pos += 4;
    }

    // TODO: Set the correct bit to 1 on rTCON, and immediately back to 0
    // (two different statements)
    // This will set the manual update bit (flushing the buffer registers)
    // and setting it to 0 again will set the timer in the ready state for start

    return 0;
}

int tmr_set_mode(enum tmr_timer t, enum tmr_mode mode) {
    int err = 0;
    int pos = t * 4;

    if (t < 0 || t > 5) {
        return -1;
    }

    // Skip dead zone
    if (t > 0) {
        pos += 4;
    }

    switch (mode) {
        case ONE_SHOT:
            // TODO: Set the appropriate bit to 0 after `pos` (the 4th bit after pos)  on rTCON
            // Check page 40 on IO docs
            break;
        case RELOAD:
            // TODO: Set the appropriate bit to 1 after `pos` (the 4th bit after pos)  on rTCON
            // Check page 40 on IO docs
            break;
        default:
            err = -1;
    }

    return err;
}

int tmr_start(enum tmr_timer t) {
    int pos = t * 4;

    if (t < 0 || t > 5) {
        return -1;
    }

    // Skip dead zone
    if (t > 0) {
        pos += 4;
    }

    // TODO: Set the start bit to 1 after `pos` (the first bit) on rTCON
    // Check page 40 on IO docs
    return 0;
}

int tmr_stop(enum tmr_timer t) {
    int pos = t * 4;

    if (t < 0 || t > 5) {
        return -1;
    }

    // Skip dead zone
    if (t > 0) {
        pos += 4;
    }

    // TODO: Set the start bit to 0 after `pos` (the first bit) on rTCON
    // Check page 40 on IO docs

    return 0;
}

// Check if the timer t is running.
// Return values:
// -1 is error (invalid timer)
// 0 is not running
// 1 is running
int tmr_isrunning(enum tmr_timer t) {
    int pos = t * 4;

    if (t < 0 || t > 5) {
        return -1;
    }

    // Skip dead zone
    if (t > 0) {
        pos += 4;
    }

    // Is the start bit set to 1?
    if ((rTCON & (0x1 << pos))) {
        return 1;
    }

    return 0;
}
