#include "cmd.h"

#include <StellarisWare/inc/hw_types.h>
#include <StellarisWare/inc/hw_memmap.h>
#include <StellarisWare/inc/hw_ints.h>
#include <StellarisWare/driverlib/pin_map.h>
#include <StellarisWare/driverlib/gpio.h>
#include <StellarisWare/driverlib/uart.h>
#include <StellarisWare/driverlib/sysctl.h>
#include <StellarisWare/driverlib/interrupt.h>


struct cmd_module cmd_modules[] = {
    {{ 0 }}, // Undefined currently
    {{ UART1_BASE, 115200, 
       SYSCTL_PERIPH_GPIOC, SYSCTL_PERIPH_UART1,
       GPIO_PC4_U1RX, GPIO_PC5_U1TX,
       GPIO_PORTC_BASE, GPIO_PIN_4 | GPIO_PIN_5,
       INT_UART1 }},
};


void cmd_init(struct cmd_module *m) {
    // Initialize UART
    SysCtlPeripheralEnable(m->PERIPH_GPIO);
    SysCtlPeripheralEnable(m->PERIPH_UART);

    // configure pin muxing
    GPIOPinConfigure(m->PRX);
    GPIOPinConfigure(m->PTX);
    GPIOPinTypeUART(m->PORT, m->PINS);

    // setup uart configuration
    UARTConfigSetExpClk(m->BASE, SysCtlClockGet(), m->BAUD,
                        (UART_CONFIG_PAR_NONE |
                         UART_CONFIG_STOP_ONE |
                         UART_CONFIG_WLEN_8));

    UARTFIFOLevelSet(m->BASE, UART_FIFO_TX1_8, UART_FIFO_RX1_8);

    // enable uart interrupts
    UARTIntEnable(m->BASE, UART_INT_RX);
    IntEnable(m->INT);
    IntPrioritySet(m->INT, 0x7f);

    UARTEnable(m->BASE);
}

void cmd_register(struct cmd_module *m, data_t cmd, 
                  void (*handler)(data_t)) {
    m->handlers[cmd] = handler;
}

void cmd_error_register(struct cmd_module *m, 
                        void (*handler)(enum cmd_error)) {
    m->error_handler = handler;
}

void cmd_send(struct cmd_module *m, data_t cmd, data_t data) {
    int i;
    data_t message[CMD_SIZE];

    message[0] = '=';   // start byte
    message[1] = cmd;   // command id
    message[2] = data;  // data

    // calculate xor check
    data_t check = 0;

    for (i = 0; i < CMD_SIZE-1; i++) {
        check ^= message[i];
    }

    message[CMD_SIZE-1] = check;

    // send message
    for (i = 0; i < CMD_SIZE; i++) {
        UARTCharPut(m->BASE, message[i]);
    }
}

void cmd_handler(struct cmd_module *m) {
    UARTIntClear(m->BASE, UART_INT_RX);

    while (true) {
        int data = UARTCharGetNonBlocking(m->BASE);

        if (data < 0 || (m->count == 0 && data != '='))
            return;

        m->buffer[m->count++] = data;

        if (m->count == CMD_SIZE) {
            m->count = 0;
            break;
        }
    }


    int i;
    void (*handler)(data_t) = m->handlers[m->buffer[1]];
    data_t data = m->buffer[2];
    data_t check = 0;

    for (i = 0; i < CMD_SIZE; i++) {
        check ^= m->buffer[i];
    }

    if (check != 0)
        return m->error_handler(CMD_BAD_CHECK);

    if (!handler)
        return m->error_handler(CMD_NO_HANDLER);

    return handler(data);
}

