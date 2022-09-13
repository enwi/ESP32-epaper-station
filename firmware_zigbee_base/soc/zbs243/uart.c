#include "uart.h"

#include "cpu.h"

void uartInit(void) {
    // clock it up
    CLKEN |= 0x20;

    // configure
    UARTBRGH = 0x00;    // at the speeds we want, this reg is always 0
    //UARTBRGL = 0x8A;  // 115200
    UARTBRGL = 69;      // nice. 230400 baud
    //UARTBRGL = 0x1F;  // 500kbaud, unreliable RX here...
    //UARTBRGL = 39;      //400kbaud? also unreliable...
    //UARTBRGL = 49;      // 320kbaud? nope. bummer.
    UARTSTA = 0x12;  // also set the "empty" bit else we wait forever for it to go up
    IEN_UART0 = 1;
}

volatile uint8_t txtail = 0;
volatile uint8_t txhead = 0;
uint8_t __xdata txbuf[256] = {0};

volatile uint8_t rxtail = 0;
volatile uint8_t rxhead = 0;
uint8_t __xdata rxbuf[256] = {0};

void uartTx(uint8_t val) {
    __critical {
        txbuf[txhead] = val;
        if (txhead == txtail) {
            UARTBUF = val;
        }
        txhead++;
    }
}

uint8_t uartRx() {
    if (rxhead == rxtail) {
        return 0;
    } else {
        uint8_t ret = rxbuf[rxtail];
        rxtail++;
        return ret;
    }
}

uint8_t uartBytesAvail() {
    return rxhead - rxtail;
}

void UART_IRQ1(void) __interrupt(0) {
    if (UARTSTA & 1) {  // RXC
        UARTSTA &= 0xfe;
        rxbuf[rxhead] = UARTBUF;
        rxhead++;
    }
    if (UARTSTA & 2) {  // TXC
        UARTSTA &= 0xfd;
        txtail++;
        if (txhead != txtail) {
            UARTBUF = txbuf[txtail];
        }
    }
}