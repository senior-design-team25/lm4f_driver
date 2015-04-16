#include "cmd.h"

#include <RASLib/inc/common.h>
#include <RASLib/inc/gpio.h>
#include <RASLib/inc/time.h>
#include <RASLib/inc/servo.h>

#include <StellarisWare/inc/hw_memmap.h>
#include <StellarisWare/inc/hw_ints.h>
#include <StellarisWare/driverlib/pin_map.h>
#include <StellarisWare/driverlib/gpio.h>
#include <StellarisWare/driverlib/uart.h>
#include <StellarisWare/driverlib/sysctl.h>
#include <StellarisWare/driverlib/interrupt.h>

#define CMD_UART UART0_BASE


// Blink the LED to show we're on
void heartbeat(void) {
    static int on = true;
    SetPin(PIN_F3, on);
    on = !on;
}

void error_handler(enum cmd_error error, data_t *message) {
    static int on = true;
    SetPin(PIN_F2, on);
    on = !on;

    Printf("bad message {%02x %02x %02x %02x}\n",
           message[0], message[1], message[2], message[3]);
}

tServo *motor;
tServo *brake;

void motor_handler(data_t val) {
    SetServo(motor, ((signed char)val)/255.0f);
}

void brake_handler(data_t val) {
    if (val > 0x7f)
        motor_handler(0);

    SetServo(brake, 1.0f - (val/255.0f));
}

// The 'main' function is the entry point of the program
int main(void) {
    // Initialization code can go here
    CallEvery(heartbeat, 0, 0.5);
    motor = InitializeServo(PIN_D0);
    motor_handler(0);
    brake = InitializeServo(PIN_D1);
    brake_handler(0);

    cmd_init();
    cmd_error_register(error_handler);

    cmd_register('m', motor_handler);
    cmd_register('b', brake_handler);

    while (1) {
        Wait(0.1f);
        cmd_send('t', 0x00);
    }
}
