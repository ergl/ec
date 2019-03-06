
#include "44b.h"
#include "intcontroller.h"

// Default controller configuration
void ic_init(void) {
    // INTMOD is Interrupt Mode Register
    // Reset to default value (0b00...00)
    // All interrupts are handled as IRQ
    rINTMOD = 0x0;

    // INTCON is the control register for the controller
    // Reset to default value (0b0111)
    // Non-vectorized mode
    // IRQ and FIQ disabled
    rINTCON = 0x7;

    // INTMSK is Interrupt Mask Register
    // Reset value is 0x07FF_FFFF, but all 1s is ok too
    // Bit per-line. When set to 1, line is masked
    rINTMSK = ~(0x0);
}

// Configure IRQ mode (enable/disable, vector/non-vector)
int ic_conf_irq(enum enable st, enum int_vec vec) {
    int conf = rINTCON;

    if (st != ENABLE && st != DISABLE) {
        return -1;
    }

    if (vec == VEC) {
        // Set 3rd bit to 0
        conf &= ~(0x4);
    } else {
        // Set 3rd bit to 1
        conf |= 0x4;
    }

    if (st == ENABLE) {
        // Set 2nd bit to 0
        conf &= ~(0x2);
    } else {
        // Set 2nd bit to 1
        conf |= 0x2;
    }

    rINTCON = conf;
    return 0;
}

// Enable/disable FIQ line.
// IMPORTANT: Disable IRQ vectorized mode before enabling FIQ
// From docs INTCON[0]: 0 = FIQ interrupt enable (Not allowed vect. int. mode)
int ic_conf_fiq(enum enable st) {
    int conf = rINTCON;

    if (st != ENABLE && st != DISABLE) {
        return -1;
    }

    if (st == ENABLE) {
        // Set 1st bit to 0
        conf &= ~(0x1);
    } else {
        // Set 1st bit to 1
        conf |= 0x1;
    }

    rINTCON = conf;
    return 0;
}

// Config interrupt line mode (IRQ/FIQ)
int ic_conf_line(enum int_line line, enum int_mode mode) {
    unsigned int bit = INT_BIT(line);

    if (line < 0 || line > 26) {
        return -1;
    }

    // Non-recognized mode
    if (mode != IRQ && mode != FIQ) {
        return -1;
    }

    // Have to change specified bit on rINTMOD
    if (mode == IRQ) {
        // Set bit to 0 (IRQ mode)
        rINTMOD &= ~bit;
    } else {
        // Set bit to 1 (FIQ mode)
        rINTMOD |= bit;
    }

    return 0;
}

// Enable (unmask) the given line
int ic_enable(enum int_line line) {
    unsigned int bit = INT_BIT(line);

    if (line < 0 || line > 26) {
        return -1;
    }

    // Set line bit to 0 to mark it as available
    rINTMSK &= ~bit;

    return 0;
}

// Disable (mask) the given line
int ic_disable(enum int_line line) {
    unsigned int bit = INT_BIT(line);

    if (line < 0 || line > 26) {
        return -1;
    }

    // Set line bit to 0 to mark it as masked
    rINTMSK |= bit;

    return 0;
}

// Clear the Service Pending for the given line
int ic_cleanflag(enum int_line line) {
    int bit;

    if (line < 0 || line > 26) {
        return -1;
    }

    bit = INT_BIT(line);

    // On {I,F}_ISPC, the flag can be cleared
    // by just writing `1` to the corresponding
    if (rINTMOD & bit) { // Line is in FIQ mode
        rF_ISPC = bit;
    } else { // Line is in IRQ mode
        rI_ISPC = bit;
    }

    return 0;
}
