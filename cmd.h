/*
 * Simple command dispatch system.
 *
 * Commands can contain a 32-bit float payload and 8-bit 
 * xorred checksum. The following is the full encoding
 *
 * [- 8 -|------ 32 ------|- 8 -]
 * [ cmd |      value     |check]
 *
 * In the case a command is dropped the registered
 * cmd error handler is called for diagnostics with 
 * the specific error number.
 *
 */

#ifndef CMD_H
#define CMD_H

// Note: if you change the base cmd_init
// must be modified accordingly
#define CMD_BASE UART1_BASE
#define CMD_BAUD 115200
#define CMD_SIZE 6

enum cmd_error {
    CMD_NO_ERROR = 0,
    CMD_BAD_CHECK,
    CMD_NO_HANDLER,
};


void cmd_init(void);

void cmd_register(unsigned char cmd, void (*handler)(float));
void cmd_error_register(void (*handler)(enum cmd_error));

void cmd_send(unsigned char cmd, float value);

#endif // CMD_H
