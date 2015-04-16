/*
 * Simple command dispatch system.
 *
 * Commands are 32-bit long and can send an 8-bit float 
 * payload to an 8-bit address. The following is the full 
 * encoding
 *
 * [- 8 -|- 8 -|- 8 -|- 8 -]
 * [ '=' | cmd | dat | xor ]
 *
 * In the case a command is dropped the registered
 * cmd error handler is called for diagnostics with 
 * the specific error number.
 *
 */

#ifndef CMD_H
#define CMD_H

#define VEHICLE_ID 0
#define DEVICE_ID 'b'

// Note: if you change the base cmd_init
// must be modified accordingly
#define CMD_BASE UART1_BASE
#define CMD_BAUD 115200
#define CMD_SIZE 4

// Definition of data element
typedef unsigned char data_t;

enum cmd_error {
    CMD_NO_ERROR = 0,
    CMD_BAD_CHECK,
    CMD_NO_HANDLER,
};


void cmd_init(void);

void cmd_register(data_t cmd, void (*handler)(data_t));
void cmd_error_register(void (*handler)(enum cmd_error, data_t *));

void cmd_send(data_t cmd, data_t value);

#endif // CMD_H
