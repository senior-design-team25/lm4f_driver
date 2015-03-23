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

void error_handler(enum cmd_error error) {
    static int on = true;
    SetPin(PIN_F2, on);
    on = !on;
}

void debug_handler(float val) {
    Printf("recv {%08x} (%f)\n", *(unsigned int *)&val, val);
}

tServo *motor;
tServo *brake;

void motor_handler(float val) {
    SetServo(motor, (1.0f+val)/2.0f);
}

void brake_handler(float val) {
    if (val > 0.0f)
        motor_handler(0.0f);

    SetServo(brake, 1.0f - val);
}

// The 'main' function is the entry point of the program
int main(void) {
    // Initialization code can go here
    CallEvery(heartbeat, 0, 0.5);
    motor = InitializeServo(PIN_D0);
    motor_handler(0.0f);
    brake = InitializeServo(PIN_D1);
    brake_handler(0.0f);

    cmd_init();
    cmd_error_register(error_handler);

    cmd_register('d', debug_handler);
    cmd_register('m', motor_handler);
    cmd_register('b', brake_handler);
}
