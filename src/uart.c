#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include "44b.h"
#include "uart.h"
#include "intcontroller.h"

#define BUFLEN 100

// Estructura utilizada para mantener el estado de cada puerto
struct port_stat {
    // Receiving mode (DIS(abled) | POLL | INT | DMA)
    enum URxTxMode rxmode;
    // Sending mode (DIS(abled) | POLL | INT | DMA)
    enum URxTxMode txmode;
    // Inbound buffer (used only on INTerrupt mode)
    // Treated as a ring buffer
    unsigned char ibuf[BUFLEN];
    // Read pointer into ibuf
    int rP;
    // Write pointer into ibuf
    int wP;
    // On INTerrupt mode, points to the string being sent
    char *sendP;
    // Should echo back received chars?
    enum ONOFF echo;
};

// Board has two UART ports
static struct port_stat uport[2];

// ISR functions for receive/send
void Uart0_RxInt(void) __attribute__ ((interrupt ("IRQ")));
void Uart0_TxInt(void) __attribute__ ((interrupt ("IRQ")));
void Uart1_RxInt(void) __attribute__ ((interrupt ("IRQ")));
void Uart1_TxInt(void) __attribute__ ((interrupt ("IRQ")));

void uart_init(void) {
    int i;

    // Init both ports
    for (i=0; i < 2; i++) {
        uport[i].rxmode = DIS;
        uport[i].txmode = DIS;
        uport[i].rP = 0;
        uport[i].wP = 0;
        uport[i].sendP = NULL;
        uport[i].echo = OFF;
    }

    // If set on INTerrupt mode, we need to register our ISR
    pISR_URXD0 = (int) Uart0_RxInt;
    pISR_UTXD0 = (int) Uart0_TxInt;
    pISR_URXD1 = (int) Uart1_RxInt;
    pISR_UTXD1 = (int) Uart1_TxInt;

    // UART_0 rx/tx lines
    ic_conf_line(INT_URXD0, IRQ);
    ic_conf_line(INT_UTXD0, IRQ);

    // UART_1 rx/tx lines
    ic_conf_line(INT_URXD1, IRQ);
    ic_conf_line(INT_UTXD1, IRQ);
}

// Setup given uart port with the ulconf and the appropriate GPIO ports
// RX and TX lines should be set as functional pins, outputting the signal
// into the DB9 connectors of the board
int uart_lconf(enum UART port, struct ulconf *lconf) {
    int baud_rate;
    // Will mask here all the values, then store directly on ULCONn
    unsigned int confvalue = 0;

    baud_rate = (int)( MCLK /(16.0 * lconf->baud) + 0.5) - 1;

    if (lconf->ired == ON) {
        confvalue |= 0x1 << 6;
    }

    switch (lconf->par) {
        case NONE:
            // Nothing to do, value is 000
            break;
        case ODD:
            confvalue |= 0b100 << 3;
            break;
        case EVEN:
            confvalue |= 0b101 << 3;
            break;
        case FONE:
            confvalue |= 0b110 << 3;
            break;
        case FZERO:
            confvalue |= 0b111 << 3;
            break;
        default:
            return -1;
    }

    // Set stop bit
    // 0 -> 1 stop bit
    // 1 -> 2 stop bit
    // If stopb == ONE, nothing to do, ONE is the default (0)
    if (lconf->stopb == TWO) {
        confvalue |= 0x1 << 2;
    }

    switch(lconf->wordlen) {
        case FIVE:
            // nothing to do, this is 00
            break;
        case SIX:
            confvalue |= 0x1;
            break;
        case SEVEN:
            confvalue |= 0x2;
            break;
        case EIGHT:
            confvalue |= 0x3;
            break;
        default:
            return -1;
    }

    switch (port) {
        case UART0:
            rULCON0 = confvalue;
            rUBRDIV0 = baud_rate;
            // Enable signal output mode in Port E
            // rPCONE[5:4] is RxD0, rPCONE[3:2] is TxD0
            // Set them to 0, then flip the bits to 10
            rPCONE = (rPCONE & ~(0xF << 2)) | (0x2 << 2) | (0x2 << 4);
            break;

        case UART1:
            rULCON1 = confvalue;
            rUBRDIV1 = baud_rate;
            // Enable signal output mode in Port C
            // rPCONC[27:26] is RxD1, rPCONC[25:24] is TxD1
            // writing 0b1111 to pos 24 writes to both at once
            rPCONC = rPCONC | (0xF << 24);
            break;

        default:
            return -1;
    }

    uport[port].echo = lconf->echo;

    return 0;
}

// Configure the given port tx mode
int uart_conf_txmode(enum UART port, enum URxTxMode mode) {
    int conf = 0;

    if (mode < 0 || mode > 3)
        return -1;

    if (port < 0 || port > 1)
        return -1;

    switch (mode) {
        case POLL: // fallthrough
        case INT:
            // POLL/INT mode is on [3:2], 01 => int/polling
            conf = 0x1;
            break;
        case DMA:
            // DMA mode is on UCONn[3:2], 10 => BDMA0; 11 => BDMA1
            conf = (port == UART0) ? 0x2 : 0x3;
            break;
        default:
            conf = 0;
    }


    // Clear bits [3:2], then set them to `conf`
    // Set rUCONn[9] to 1 for tx interrupt mode level (0 is Pulse)
    switch (port) {
        case UART0:
            rUCON0 = (rUCON0 & ~(0x3 << 2)) | (conf << 2) | (0x1 << 9);
            break;

        case UART1:
            rUCON1 = (rUCON1 & ~(0x3 << 2)) | (conf << 2) | (0x1 << 9);
            break;
    }

    uport[port].txmode = mode;

    return 0;
}

// Configure the given port rx mode
int uart_conf_rxmode(enum UART port, enum URxTxMode mode) {
    int conf = 0;

    if (mode < 0 || mode > 3)
        return -1;

    if (port < 0 || port > 1)
        return -1;

    switch (mode) {
        case POLL: // fallthrough
        case INT:
            conf = 0x1;
            break;
        case DMA:
            conf = (port == UART0) ? 0x2 : 0x3;
            break;
        default:
            conf = 0;
    }

    // Clear bits [1:0], then set them to `conf`
    // Clear rUCONn[0] (0) for rx interrupt mode pulse (1 is level)
    // Also, if mode is interrupt, enable the line
    switch (port) {
        case UART0:
            rUCON0 = (rUCON0 & ~((0x1 << 8) | 0x3)) | conf;
            if (mode == INT) {
                ic_enable(INT_URXD0);
            }
            break;

        case UART1:
            rUCON1 = (rUCON1 & ~((0x1 << 8) | 0x3)) | conf;
            if (mode == INT) {
                ic_enable(INT_URXD1);
            }
            break;
    }

    uport[port].rxmode = mode;

    return 0;
}

// Blocking function until the given port has received data
static void uart_rx_ready(enum UART port) {
    // rUTRSTATn[0] is set to 1
    // when the received buffer has received data
    // XXX(borja): Only valid if FIFO is disabled
    // If enabled, check rUFSTATn
    switch (port) {
        case UART0:
            while ((rUTRSTAT0 & 1) == 0);
            break;

        case UART1:
            while ((rUTRSTAT1 & 1) == 0);
            break;
    }
}

// Blocking function until the given port has sent data
// (tx buffer empty)
static void uart_tx_ready(enum UART port) {
    // rUTRSTATn[1] is set to 1
    // when the transmit buffer is empty
    // XXX(borja): Only valid if FIFO is disabled
    // If enabled, check rUFSTATn
    switch (port) {
        case UART0:
            while ((rUTRSTAT0 & 2) == 0);
            break;

        case UART1:
            while ((rUTRSTAT1 & 2) == 0);
            break;
    }
}

// Write the given char in the BDMA zone (DMA) / register (INT/POLL) of the port
static void uart_write(enum UART port, char c) {
    if (port == UART0) {
        WrUTXH0(c);
    } else {
        WrUTXH1(c);
    }
}

// Read directly off BDMA zone (DMA) / register (INT/POLL) of the port
// If echo is enabled, immediately transmit it back
static char uart_read(enum UART port) {
    char c;

    if (port == UART0) {
        c = RdURXH0();
    } else {
        c = RdURXH1();
    }

    if (uport[port].echo == ON) {
        // Block until tx is ready
        uart_tx_ready(port);
        uart_write(port, c);
    }

    return c;
}

// Read from the port into the ring buffer
// Called by Rx ISR, so no need to wait for data on buffer,
// we already know it's there.
//
// Will block if echo mode is enabled, wait till tx is ready
static void uart_readtobuf(enum UART port) {
    char c;
    struct port_stat *pst = &uport[port];

    // Read from port (this function will block on echo)
    // and write it to the ring
    c = uart_read(port);
    pst->ibuf[pst->wP] = c;
    pst->wP = (pst->wP + 1) % BUFLEN;
}

// Read from the ring buffer
// Block until we've put at least one value into the ring buffer
// (this is only called on interrupt mode, the ISR will put a character
// into the ring buffer)
static char uart_readfrombuf(enum UART port) {
    char data;
    struct port_stat *pst = &uport[port];

    uart_rx_ready(port);
    data = pst->ibuf[pst->rP];
    pst->rP = (pst->rP + 1) % BUFLEN;
    return data;
}

// Rx ISR on port 0
// This interrupt is raised whenever
// the receive shift register is filled with data
void Uart0_RxInt(void) {
    // Write it to the ring buffer
    uart_readtobuf(UART0);
    ic_cleanflag(INT_URXD0);
}

// Rx ISR on port 1
// This interrupt is raised whenever
// the receive shift register is filled with data
void Uart1_RxInt(void) {
    uart_readtobuf(UART1);
    ic_cleanflag(INT_URXD1);
}

// Called by the Tx ISR. Should send the port_start->sendP
// string one byte at a time.
// As soon as the entire string is sent, disable interrupts and signal
// caller that we're done.
static void uart_dotxint(enum UART port) {
    enum int_line target_line;
    struct port_stat *pst = &uport[port];

    if (*pst->sendP != '\0' ) {
        if (*pst->sendP == '\n') {
            // \n -> \r\n conversion for windows
            // Then block until tx is ready again
            // (this would raise an interrupt, but we don't care)
            uart_write(port, '\r');
            uart_tx_ready(port);
        }

        uart_write(port, *pst->sendP);
        pst->sendP++;
    }

    // When we're done, disable Tx interrupts, and signal caller
    // by flipping the send char array to NULL
    if (*pst->sendP == '\0') {
        target_line = (port == UART0) ? INT_UTXD0 : INT_UTXD1;
        ic_disable(target_line);
        pst->sendP = NULL;
    }
}

// Tx ISR on port 0
// This interrupt is raised whenever
// the transmit shift register is flushed
void Uart0_TxInt(void) {
    uart_dotxint(UART0);
    ic_cleanflag(INT_UTXD0);
}

// Tx ISR on port 1
// This interrupt is raised whenever
// the transmit shift register is flushed
void Uart1_TxInt(void) {
    uart_dotxint(UART1);
    ic_cleanflag(INT_UTXD1);
}

// Blocking function, reads from UARt port into c
int uart_getch(enum UART port, char *c) {
    if (port < 0 || port > 1) {
        return -1;
    }

    switch (uport[port].rxmode) {
        case POLL:
            // If poll, wait until rx ready, then read from shifter
            uart_rx_ready(port);
            *c = uart_read(port);
            break;

        case INT:
            // If interrupt mode, read from the ring buffer
            // that the ISR will put there
            *c = uart_readfrombuf(port);
            break;

        case DMA:
            // TODO(borja): Optional
            return -1;
            break;

        default:
            return -1;
    }

    return 0;
}

// Blocking send the given char
int uart_sendch(enum UART port, char c) {
    // Used in interrupt mode, a valid C string (with \0 at the end)
    char localB[2] = {0};

    if (port < 0 || port > 1) {
        return -1;
    }


    switch (uport[port].txmode) {
        case POLL:
            // Wait until Tx is ready, then send `c`
            uart_tx_ready(port);
            // Under windows, perform \n -> \r\n conversion
            if (c == '\n') {
                uart_write(port, '\r');
                uart_tx_ready(port);
            }
            uart_write(port, c);
            break;

        case INT:
            // Under interrupt mode, send a one-byte string (plus \0)
            localB[0] = c;
            uart_send_str(port, localB);
            break;

        case DMA:
            // TODO(borja): Optional
            return -1;
            break;

        default:
            return -1;
    }

    return 0;
}

// Blocking send the given string (ends in \0)
int uart_send_str(enum UART port, char *str) {
    enum int_line target_line;
    struct port_stat *pst = &uport[port];

    if (port < 0 || port > 1) {
        return -1;
    }


    switch (uport[port].txmode) {
        case POLL:
            // Send the string byte by byte
            while (*str != '\0') {
                uart_sendch(port, *str);
                str++;
            }
            break;

        case INT:
            // Point to the sender buffer, then wait until the ISR
            // sends all the bytes
            // The ISR will set sendP to NULL once it's done
            pst->sendP = str;
            target_line = (port == UART0) ? INT_UTXD0 : INT_UTXD1;
            ic_enable(target_line);
            while(pst->sendP != NULL);
            break;

        case DMA:
            // TODO(borja): Optional
            return -1;
            break;

        default:
            return -1;
    }

    return 0;

}

// Send a printf-ed string to the port (blocking)
void uart_printf(enum UART port, char *fmt, ...) {
    va_list ap;
    char str[256];

    va_start(ap, fmt);
    vsnprintf(str, 256, fmt, ap);
    uart_send_str(port, str);
    va_end(ap);
}
