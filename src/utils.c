/*-------------------------------------------------------------------
**
**  Fichero:
**    utils.c  10/6/2014
**
**    Estructura de Computadores
**    Dpto. de Arquitectura de Computadores y Autom�tica
**    Facultad de Inform�tica. Universidad Complutense de Madrid
**
**  Prop�sito:
**    Contiene las implementaciones de funciones auxiliares
**
**  Notas de dise�o:
**    Equivale al fichero hom�nimo usado en Fundamentos de Computadores
**
**-----------------------------------------------------------------*/

#include "44b.h"

//--------------------------------SYSTEM---------------------------------//
static int delayLoopCount=400;

// time=0: adjust the Delay function by WatchDog timer.//
// time>0: the number of loop time//
// 100us resolution.//
void Delay(int time) {
    int i,adjust=0;
    if (time==0) {
        time=200;
        adjust=1;
        delayLoopCount=400;
        rWTCON=((MCLK/1000000-1)<<8)|(2<<3);	// 1M/64,Watch-dog,nRESET,interrupt disable//
        rWTDAT=0xffff;
        rWTCNT=0xffff;
        rWTCON=((MCLK/1000000-1)<<8)|(2<<3)|(1<<5); // 1M/64,Watch-dog enable,nRESET,interrupt disable //
    }
    for(;time>0;time--)
        for(i=0;i<delayLoopCount;i++);
    if(adjust==1)
    {
        rWTCON=((MCLK/1000000-1)<<8)|(2<<3);
        i=0xffff-rWTCNT;   //  1count/16us?????????//
        delayLoopCount=8000000/(i*64);	//400*100/(i*64/200)   //
    }
}

// Modulo for both positive and negative numbers
int modulo(int x, int n) {
    return ((x % n) + n) % n;
}

// Translates the given ASCII code to a normal digit
// Returns -1 if `c` is not a valid alphanumeric codepoint
char ascii2digit(char c) {
    char d = -1;
    if ((c >= '0') && (c <= '9')) {
        d = c - '0';
    } else if ((c >= 'a') && (c <= 'f')) {
        d = c - 'a' + 10;
    } else if ((c >= 'A') && (c <= 'F')) {
        d = c - 'A' + 10;
    }

    return d;
}
