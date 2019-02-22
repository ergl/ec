/*-------------------------------------------------------------------
**
**  Fichero:
**    button.c  10/6/2014
**
**    Estructura de Computadores
**    Dpto. de Arquitectura de Computadores y Automática
**    Facultad de Inform�tica. Universidad Complutense de Madrid
**
**  Propósito:
**    Contiene las implementaciones de las funciones
**    para la gestión de los pulsadores de la placa de prototipado
**
**  Notas de diseño:
**
**-----------------------------------------------------------------*/

#include "44b.h"
#include "utils.h"
#include "button.h"
#include "leds.h"
#include "gpio.h"

// Button pins
#define PIN_BTN1 6
#define PIN_BTN2 7

// Read button status
// Return value has the last two bits
// encoding button state
//
// If the bit is 1, the button was pressed
// If the bit is 0, the button was not pressed
unsigned int read_button(void) {
    unsigned int buttons = 0;
    enum digital val;

    // Read from button 1
    if (portG_read(PIN_BTN1, &val) != 0) {
        return -1;
    }

    if (val == LOW) {
        buttons |= BUT1;
    }

    if (portG_read(PIN_BTN2, &val) != 0) {
        return -1;
    }

    if (val == LOW) {
        buttons |= BUT2;
    }

    return buttons;
}
