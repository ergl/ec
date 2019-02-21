#include "44b.h"
#include "gpio.h"

// Port B interface implementation
int portB_conf(int pin, enum port_mode mode) {
    int ret = 0;

    if (pin < 0 || pin > 10) {
        return -1;
    }

    if (mode == SIGOUT) {
        // TODO(borja): Implement
        // Set on rPCONB the correct bit to 1 to set as signal output pin
        // See page 5 of class notes
    } else if (mode == OUTPUT) {
        // TODO(borja): Implement
        // Set on rPCONB the correct bit to 1 to set as output pin
        // See page 5 of class notes
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
        // TODO(borja): Implement
        // Set on rPDATB the correct bit for the given pin to 1
    } else {
        // TODO(borja): Implement
        // Set on rPDATB the correct bit for the given pin to 0
    }

    return 0;
}

// Port G interface implementation
int portG_conf(int pin, enum port_mode mode) {
    int pos = pin*2;

    if (pin < 0 || pin > 7) {
        return -1;
    }

    // TODO(borja): Implement
    // See page 5 on class notes
    switch (mode) {
        case INPUT:
            // Write 00 to rPCONG[pos:pos+1] to conf as input
            break;
        case OUTPUT:
            // Write 01 to rPCONG[pos:pos+1] to conf as output
            break;
        case SIGOUT:
            // Write 10 to rPCONG[pos:pos+1] to conf as signal output
            break;
        case EINT:
            // Write 11 to rPCONG[pos:pos+1] to conf as interrupt gen
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

    if ((rPCONG & (0x3 << pos)) != (0x1 << pos)) {
        return -1; // indica error
    }

    if (val) {
        // Set on rPDATG the correct bit for the given pin to 1
    } else {
        // Set on rPDATG the correct bit for the given pin to 0
    }

    return 0;
}

// TODO(borja): Implement
// See page 6 on class notes
int portG_read(int pin, enum digital* val) {
    int pos = pin*2;

    if (pin < 0 || pin > 7) {
        return -1;
    }

    if (rPCONG & (0x3 << pos)) {
        return -1;
    }

    // TODO(borja): true if in rPDATG, the given pin is set to 1
    if (/* XXX */) {
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

    // TODO(borja): Implement
    if (st == ENABLE) {
        // Set `pin` on rPUPG to the appropriate value to enable pull-up
    } else {
        // Set `pin` on rPUPG to the appropriate value to disable pull-up
    }

    return 0;
}

