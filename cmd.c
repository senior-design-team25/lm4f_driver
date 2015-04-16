#include "cmd.h"

#include <StellarisWare/inc/hw_types.h>
#include <StellarisWare/inc/hw_memmap.h>
#include <StellarisWare/inc/hw_ints.h>
#include <StellarisWare/driverlib/pin_map.h>
#include <StellarisWare/driverlib/gpio.h>
#include <StellarisWare/driverlib/uart.h>
#include <StellarisWare/driverlib/sysctl.h>
#include <StellarisWare/driverlib/interrupt.h>


// handler functions for dispatching calls
// defaults to null pointers and can be set
// to remove handlers
static void (*cmd_handlers[256])(data_t);

static void (*cmd_error_handler)(enum cmd_error, data_t *);

// buffer for recieved messages
static unsigned int cmd_count;
static unsigned char cmd_buffer[CMD_SIZE];


void cmd_init(void) {
    // Initialize UART
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOC);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART1);

    // configure pin muxing
    GPIOPinConfigure(GPIO_PC4_U1RX);
    GPIOPinConfigure(GPIO_PC5_U1TX);
    GPIOPinTypeUART(GPIO_PORTC_BASE, GPIO_PIN_4 | GPIO_PIN_5);

    // setup uart configuration
    UARTConfigSetExpClk(CMD_BASE, SysCtlClockGet(), CMD_BAUD,
                        (UART_CONFIG_PAR_NONE |
                         UART_CONFIG_STOP_ONE |
                         UART_CONFIG_WLEN_8));

    UARTFIFOLevelSet(CMD_BASE, UART_FIFO_TX1_8, UART_FIFO_RX1_8);

    // enable uart interrupts
    UARTIntEnable(CMD_BASE, UART_INT_RX);
    IntEnable(INT_UART1);
    IntPrioritySet(INT_UART1, 0x7f);

    UARTEnable(CMD_BASE);
}

void cmd_register(data_t cmd, void (*handler)(data_t)) {
    cmd_handlers[cmd] = handler;
}

void cmd_error_register(void (*handler)(enum cmd_error, data_t *)) {
    cmd_error_handler = handler;
}

void cmd_send(data_t cmd, data_t data) {
    int i;
    data_t message[CMD_SIZE];

    message[0] = '=';           // start byte
    message[1] = cmd;           // command id
    message[2] = data;          // data

    // calculate xor check
    data_t check = 0;

    for (i = 0; i < CMD_SIZE-1; i++) {
        check ^= message[i];
    }

    message[CMD_SIZE-1] = check;

    // send message
    for (i = 0; i < CMD_SIZE; i++) {
        UARTCharPut(CMD_BASE, message[i]);
    }
}

void cmd_handler(void) {
    UARTIntClear(CMD_BASE, UART_INT_RX);

    while (true) {
        int data = UARTCharGetNonBlocking(CMD_BASE);

        if (data < 0 || (cmd_count == 0 && data != '='))
            return;

        cmd_buffer[cmd_count++] = data;

        if (cmd_count == CMD_SIZE) {
            cmd_count = 0;
            break;
        }
    }


    int i;
    void (*handler)(data_t) = cmd_handlers[cmd_buffer[1]];
    data_t data = cmd_buffer[2];
    data_t check = 0;

    for (i = 0; i < CMD_SIZE; i++) {
        check ^= cmd_buffer[i];
    }

    if (check != 0)
        return cmd_error_handler(CMD_BAD_CHECK, cmd_buffer);

    if (!handler)
        return cmd_error_handler(CMD_NO_HANDLER, cmd_buffer);

    return handler(data);
}

