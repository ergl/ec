/*-------------------------------------------------------------------
**
**  Fichero:
**    button.c  10/6/2014
**
**    Estructura de Computadores
**    Dpto. de Arquitectura de Computadores y Autom�tica
**    Facultad de Inform�tica. Universidad Complutense de Madrid
**
**  Prop�sito:
**    Contiene las implementaciones de las funciones
**    para la gesti�n de los pulsadores de la placa de prototipado
**
**  Notas de dise�o:
**
**-----------------------------------------------------------------*/

#include "44b.h"
#include "utils.h"
#include "button.h"
#include "leds.h"
#include "gpio.h"

unsigned int read_button(void) {
    unsigned int buttons = 0;
    enum digital val;

    // TODO(borja): Implement read_button
    // Using gpio.h API, read from pins 6 and 7 from Port G (portG_read)
    // Return value (buttons) will be an unsigned int where the last two bits
    // will encode button state
    //
    // buttons[0] -> Pin 6 status (0 is pushed, 1 is not)
    // buttons[1] -> Pin 7 status (0 is pushed, 1 is not)
    // Use BUT1 (0x1) and BUT2 (0x2) macros to return
    // BUT1 -> BUT1 was pressed
    // BUT2 -> BUT2 was pressed
    //
    // XXX: See page 6 on class notes

    return buttons;
}
