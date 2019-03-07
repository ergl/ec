
#include "44b.h"
#include "timer.h"

// Set the prescaler `p` to the given `value`
// Each `p` is shared among two timers
// p = 0 for timer 0 and 1
// p = 1 for timer 2 and 3
// p = 2 for timer 4 and 5
int tmr_set_prescaler(int p, int value) {
    int offset = p * 8;

    // Clear off everything except value[7:0],
    // just in case we get a larger value.
    // We don't want to overrun the prescaler section
    value &= 0xFF;

    if (p < 0 || p > 3) {
        return -1;
    }

    // rTCFG0 is a 32bit register, divided in 4 8-bit zones
    // The first 3 are for prescaler (LSB), the last one is for
    // dead zone configuration.
    // Need to write the given value directly on those bits
    rTCFG0 = ( (rTCFG0 & ~(0xFF << offset)) // Zero-out the target bits
             | (value << offset) // Then flip to 1 those in `value`
             );

    return 0;
}

// Set the divider `d` to the given value `div_v`
int tmr_set_divider(int d, enum tmr_div div_v) {
    int offset = d * 4;

    int invalid_timer = (d < 0 || d > 5);
    int invalid_divisor = (div_v == D1_32 && d > 3) ||
                          (div_v == EXTCLK && d != 5) ||
                          (div_v == TCLK && d != 4);

    if (invalid_timer || invalid_divisor) {
        return -1;
    }

    // Map those values to the correct int cast
    if (div_v == EXTCLK || div_v == TCLK) {
        div_v = 4;
    }

    // rTCFG1 is a 28-bit wide reg, dividied in 6 4-bit MUX zones
    // (one for each timer), and a 4-bit DMA mode zone.
    // Depending on the MUX, certain values will be valid
    // (the enum tmr_div already maps the correct values)

    // For timers 0-3:
    // 0000 (0) -> 1/2
    // 0001 (1) -> 1/4
    // 0010 (2) -> 1/8
    // 0011 (3) -> 1/16
    // 01XX (4) -> 1/32

    // For timers 4 and 5
    // 0000 (0) -> 1/2
    // 0001 (1) -> 1/4
    // 0010 (2) -> 1/8
    // 0011 (3) -> 1/16
    // 01XX (4) -> TCLK (timer 4) or EXTCLK (timer 5)

    // Have to write `div_v` at `offset` bits in rTCFG1
    rTCFG1 = ( (rTCFG1 & ~(0xF << offset)) // Zero-out the target bits
             | (div_v << offset) // Then flip to 1 those in `value`
             );

    return 0;
}

int tmr_set_count(enum tmr_timer t, int count, int cmp) {
    // Buffer registers are 16-bit wide.
    // We can write directly, the highest 16 bits will be dropped
    switch (t) {
        case TIMER0:
            rTCNTB0 = count;
            rTCMPB0 = cmp;
             break;
        case TIMER1:
            rTCNTB1 = count;
            rTCMPB1 = cmp;
             break;
        case TIMER2:
            rTCNTB2 = count;
            rTCMPB2 = cmp;
             break;
        case TIMER3:
            rTCNTB3 = count;
            rTCMPB3 = cmp;
             break;
        case TIMER4:
            rTCNTB4 = count;
            rTCMPB4 = cmp;
             break;
        case TIMER5:
            rTCNTB5 = count;
            // Timer 5 doesn't have a CMP register
             break;
        default:
            return -1;
    }

    return 0;
}

// Force-update the timer `t`
int tmr_update(enum tmr_timer t) {
    int offset = t * 4;

    if (t < 0 || t > 5) {
        return -1;
    }

    // Skip dead zone located between timer 0 and 1 (4 bits)
    if (t > 0) {
        offset += 4;
    }

    // rTCON is a 27-bit wide register divided in 5 timer zones + dead zone,
    // with the latter interleaved between the zones of timer 0 and 1.
    // In each timer zone, we get the bits
    // 0 -> start(1) / stop(0)
    // 1 -> manual update. (0) => noop (1) => do update
    // 2 -> inverter on(1) / off (0)
    // 3 -> auto reload (1) / one shot (0)

    // Flush the buffer registers (force manual update)
    rTCON |= (0x2 << offset);
    // From docs: This bit has to be cleared at next writing
    rTCON &= ~(0x2 << offset);

    return 0;
}

// Configure mode for timer `t`
int tmr_set_mode(enum tmr_timer t, enum tmr_mode mode) {
    int offset = t * 4;

    if (t < 0 || t > 5) {
        return -1;
    }

    // Skip dead zone located between timer 0 and 1 (4 bits)
    if (t > 0) {
        offset += 4;
    }

    // In each timer zone, we get the bits
    // 0 -> start(1) / stop(0)
    // 1 -> manual update. (0) => noop (1) => do update
    // 2 -> inverter on(1) / off (0)
    // 3 -> auto reload (1) / one shot (0)

    switch (mode) {
        case ONE_SHOT:
            // one-shot is located at the 4th bit
            rTCON &= ~(0b1000 << offset);
            break;
        case RELOAD:
            rTCON |= (0b1000 << offset);
            break;
        default:
           return -1;
    }

    return 0;
}

int tmr_start(enum tmr_timer t) {
    int offset = t * 4;

    if (t < 0 || t > 5) {
        return -1;
    }

    // Skip dead zone
    if (t > 0) {
        offset += 4;
    }

    // rTCON[0 + offset] -> start(1) / stop(0)
    rTCON |= (0x1 << offset);

    return 0;
}

int tmr_stop(enum tmr_timer t) {
    int offset = t * 4;

    if (t < 0 || t > 5) {
        return -1;
    }

    // Skip dead zone
    if (t > 0) {
        offset += 4;
    }

    // rTCON[0 + offset] -> start(1) / stop(0)
    rTCON &= ~(0x1 << offset);

    return 0;
}

// Check if the timer t is running.
// Return values:
// -1 is error (invalid timer)
// 0 is not running
// 1 is running
int tmr_isrunning(enum tmr_timer t) {
    int offset = t * 4;

    if (t < 0 || t > 5) {
        return -1;
    }

    // Skip dead zone
    if (t > 0) {
        offset += 4;
    }

    // Is the start bit set to 1?
    if ((rTCON & (0x1 << offset))) {
        return 1;
    }

    return 0;
}
