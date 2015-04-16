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

#define CMD_SIZE 4

// Definition of data element
typedef unsigned char data_t;

enum cmd_error {
    CMD_NO_ERROR = 0,
    CMD_BAD_CHECK,
    CMD_NO_HANDLER,
};

struct cmd_module {
    const struct {
        unsigned int BASE;
        unsigned int BAUD;

        unsigned int PERIPH_GPIO;
        unsigned int PERIPH_UART;

        unsigned int PRX;
        unsigned int PTX;

        unsigned int PORT;
        unsigned int PINS;

        unsigned int INT;
    };

    // handler functions for dispatching calls
    // defaults to null pointers and can be set
    // to remove handlers
    void (*handlers[256])(data_t);
    void (*error_handler)(enum cmd_error);

    // buffer for recieved messages
    int count;
    data_t buffer[CMD_SIZE];
};

// table of available modules
extern struct cmd_module cmd_modules[];

#define cmd0 (&cmd_modules[0])
#define cmd1 (&cmd_modules[1])


void cmd_init(struct cmd_module *m);

void cmd_register(struct cmd_module *m, data_t cmd, 
                  void (*handler)(data_t));

void cmd_error_register(struct cmd_module *m, 
                        void (*handler)(enum cmd_error));

void cmd_send(struct cmd_module *m, data_t cmd, data_t value);

void cmd_handler(struct cmd_module *m);


#endif // CMD_H
