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


// Blink the LED to show we're on
void heartbeat(void) {
    static int on = true;
    SetPin(PIN_F3, on);
    on = !on;
}

void error_handler(enum cmd_error error) {
    static int on = true;
    SetPin(PIN_F2, on);
    on = !on;

    Printf("bad message {%02x %02x %02x %02x}\n",
           cmd1->buffer[0], cmd1->buffer[1], cmd1->buffer[2], cmd1->buffer[3]);
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

    cmd_init(cmd1);
    cmd_error_register(cmd1, error_handler);

    cmd_register(cmd1, 'm', motor_handler);
    cmd_register(cmd1, 'b', brake_handler);

    while (1) {
        Wait(0.1f);
        cmd_send(cmd1, 't', 0x00);
    }
}
