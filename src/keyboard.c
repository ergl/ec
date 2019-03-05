
#include "44b.h"
#include "keyboard.h"

// No key is pressed
#define KEY_VALUE_MASK 0x0F

// Start keyboard addr
volatile static unsigned char *keyboard_base = (unsigned char *)0x06000000;

int kb_scan(void) {
    int i;
    char temp;
    int value = -1;
    // Line addresses
    // Given in 8 bit offsets after keyboard_base
    int lines[4] = {0xfd, 0xfb, 0xf7, 0xef};
    int keymap[4][4] = { {0, 1, 2, 3}
                       , {4, 5, 6, 7}
                       , {8, 9, 0xA, 0xB}
                       , {0xC, 0xD, 0xE, 0xF}
                       };

    // Check all `lines` addr
    for (i = 0; (i < 4) && (value == -1); i++) {
        temp = *(keyboard_base+lines[i]);

        // The returned value is not the 0x0F (no key pressed)
        // TODO: Should bail out after first key match?
        if ((temp & KEY_VALUE_MASK) != KEY_VALUE_MASK) {
            // TODO: Check the returned key in the appropriate bit in temp
            // | Bit (Set to 0) | Key
            // |        1       | keymap[i][3]
            // |        2       | keymap[i][2]
            // |        3       | keymap[i][1]
            // |        4       | keymap[i][0]
        }
    }

    return value;
}
