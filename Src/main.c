//PIN CONNECTIONS
//--------------
//A0  ---> EN1
//A5  --->IN3
//A6  --->ECHO
//A7  --->IN4
//BO  --->TRIGGER
//B6  --->IN1
//B7 --->IN2

#include "speed.h"
#include <stdint.h>
#include "stm32f103xb.h"
#include "direction.h"
#include "ultrasonic.h"
#include "navigation.h"

#define STOP_CM  4
#define RESUME_CM 15      // hysteresis: must clear 15 cm before driving again
#define DRIVE_CCR 800     // ~80% duty, tune to your motor
int main(void)
{
	  gpioinit();
	  timerpwm();
	  pwm2();
	  __enable_irq();
	  state_run();          // never returns


}
