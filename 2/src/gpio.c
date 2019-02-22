#include "44b.h"
#include "gpio.h"

// Port B interface implementation
int portB_conf(int pin, enum port_mode mode) {
    int ret = 0;

    if (pin < 0 || pin > 10) {
        return -1;
    }

    if (mode == SIGOUT) {
        // Set pin bit to 1 (sigout mode)
        rPCONB |= (0x1 << pin);
    } else if (mode == OUTPUT) {
        // Set pin bit to 0 (output mode)
        rPCONB &= ~(0x1 << pin);
    } else {
        ret = -1;
    }

    return ret;
}

int portB_write(int pin, enum digital val) {
    if (pin < 0 || pin > 10) {
        return -1;
    }

    if (val < 0 || val > 1) {
        return -1;
    }

    if (val) {
        // Set pin bit to 1
        rPDATB |= (0x1 << pin);
    } else {
        // Set pin bit to 0
        rPDATB &= ~ (0x1 << pin);
    }

    return 0;
}

// Port G interface implementation
int portG_conf(int pin, enum port_mode mode) {
    int pos = pin*2;

    if (pin < 0 || pin > 7) {
        return -1;
    }

    switch (mode) {
        case INPUT:
            // Write 00 to rPCONG[pos:pos+1] to conf as input
            rPCONG &= ~(0x3 << pos);
            break;
        case OUTPUT:
            // Write 01 to rPCONG[pos:pos+1] to conf as output
            // First set to 00, then flip the first
            rPCONG = ((rPCONG & ~(0x3 << pos)) | 0x1 << pos);
            break;
        case SIGOUT:
            // Write 10 to rPCONG[pos:pos+1] to conf as signal output
            // First set to 00, then flip the last
            rPCONG = ((rPCONG & ~(0x3 << pos)) | 0x1 << (pos + 1));
            break;
        case EINT:
            // Write 11 to rPCONG[pos:pos+1] to conf as interrupt gen
            rPCONG |= (0x3 << pos);
            break;

        default:
            return -1;
    }

    return 0;
}

// TODO(borja): Implement in advanced part
int portG_eint_trig(int pin, enum trigger trig) {
    return 0;
}

int portG_write(int pin, enum digital val) {
    int pos = pin*2;

    if (pin < 0 || pin > 7) {
        return -1;
    }

    if (val < 0 || val > 1) {
        return -1;
    }

    // If not in output mode (01), bail out
    if ((rPCONG & (0x3 << pos)) != (0x1 << pos)) {
        return -1;
    }

    if (val) {
        // Set on rPDATG the correct bit for the given pin to 1
        rPDATG |= (0x1 << pin);
    } else {
        // Set on rPDATG the correct bit for the given pin to 0
        rPDATG &= ~(0x1 << pin);
    }

    return 0;
}

int portG_read(int pin, enum digital* val) {
    int pos = pin*2;

    if (pin < 0 || pin > 7) {
        return -1;
    }

    // If not in input mode (00), bail out
    if (rPCONG & (0x3 << pos)) {
        return -1;
    }

    // If pin bit on rPDATG is set to 1, return HIGH
    int mask = (0x1 << pin);
    if ((rPDATG & mask) == mask) {
        *val = HIGH;
    } else {
        *val = LOW;
    }

    return 0;
}

// Configure pull-up register
int portG_conf_pup(int pin, enum enable st) {
    if (pin < 0 || pin > 7) {
        return -1;
    }

    if (st != ENABLE && st != DISABLE) {
        return -1;
    }

    // For PUPC-PUPG (pull up reg)
    // If bit is 0 -> enabled
    // If bit is 1 -> disabled
    if (st == ENABLE) {
        // Set on rPUPG the `pin` bit to 0 (enable pull-up)
        rPUPG &= ~ (0x1 << pin);
    } else {
        // Set on rPUPG the `pin` bit to 1 (disable pull-up)
        rPUPG |= (0x1 << pin);
    }

    return 0;
}

