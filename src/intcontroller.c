
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

int ic_conf_irq(enum enable st, enum int_vec vec) {
    int conf = rINTCON;

    if (st != ENABLE && st != DISABLE) {
        return -1;
    }


    if (vec == VEC) {
        // TODO: Set IRQ line in vectorized mode
    } else {
        // TODO: Set IRQ line in non-vectorized mode
    }


    if (st == ENABLE) {
        // TODO: Enable IRQ line
    } else {
        // TODO: Disable IRQ line
    }

    rINTCON = conf;
    return 0;
}

int ic_conf_fiq(enum enable st) {
    int ret = 0;

    if (st != ENABLE && st != DISABLE) {
        return -1;
    }

    if (st == ENABLE) {
        // TODO: Enable FIQ line
    } else {
        // TODO: Disable FIQ line
    }

    return 0;
}

int ic_conf_line(enum int_line line, enum int_mode mode) {
    unsigned int bit = INT_BIT(line);

    if (line < 0 || line > 26) {
        return -1;
    }

    // Non-recognized mode
    if (mode != IRQ && mode != FIQ) {
        return -1;
    }

    if (mode == IRQ) {
        // TODO: Set up line `line` on IRQ
    } else {
        // TODO: Set up line `line` on FIQ
    }

    return 0;
}

int ic_enable(enum int_line line) {
    if (line < 0 || line > 26) {
        return -1;
    }

    // TODO: Enable interrupts on line `line`

    return 0;
}

int ic_disable(enum int_line line) {
    if (line < 0 || line > 26) {
        return -1;
    }

    // TODO: Mask interrupts on line `line`

    return 0;
}

int ic_cleanflag(enum int_line line) {
    int bit;

    if (line < 0 || line > 26) {
        return -1;
    }

    bit = INT_BIT(line);

    if (rINTMOD & bit) {
        // TODO: Clean up flag on `line`, being that line in FIQ mode
    } else {
        // TODO: Clean up flag on `line`, being that line in IRQ mode
    }

    return 0;
}
