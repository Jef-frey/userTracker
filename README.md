# userTracker
This project contains the software and design to build a device that will rotate, following user's hand movement.

## Components
IR transmitters and receivers will be used to detect user's hand movement, a motor will be used such that the device follows the user's hand. A LED will be used to simulate the eye of the device. And PIC16F15344 will be used to control the signals.

- 1 x PIC16F15344
- 1 x 28BYJ-48 Stepper Motor
- 1 x yellow LED
- 2 x IR receivers
- 2 x IR transmitters
- 3 x 1 kOhm current-limiting resistor
- 2 x 10 kOhm resistor

## Schematic
A pair of IR transmitter/receiver will be used to detect how close user's hand is to the device. Two pairs will be used to compare the distance of the sensors and user's hand, and device will be mounted on a motor to rotate in the direction where user's hand is closer to the device.


## Microcontroller requirements
A different microcontroller can be used, but it have to support the following features/peripherals
- 2 timers
2 timers will be used to support the other peripherals
- comparator
- PWM
- ADC

## Software
The device has 3 states: IDLE, CW, CCW
In IDLE state, motor will stop, LED will fade off if it was on(TMR0 will slowly decrement the duty cycle of the PWM). Comparator will be used to compare voltage difference between IR receivers, the change in output will turn on the LED to start fading on(TMR0 will slowly increment the duty cycle of the PWM), and device will enter CW(clockwise) state or CCW(counter-clockwise) state depends on the comparator output.
In CW state, motor will turn clockwise, and ADC will continuously detect IR receivers' voltage level. Once both receivers' voltage level is lower than specific level for a few cycles (to avoid misread), device will enter IDLE state.
In CCW state, motor will turn counter-clockwise, everything else is same as CW state.



## Possible issues and improvements
- Different sensors to detect user's motion were originally planned, as the current detection method has very limited range. The first iteration used PIR sensor, but it cannot be used on moving devices and has a ~2 sec delay time after motions stopped so it wasn't used. Upon further research time of flight sensor may be used, but it is more costly.
- ADC voltage level at which device switches back to IDLE state may need to be changed based on where the IR sensors are located.

ir transmitter PWM
if transmitter is far enough comparator can be used 
sleep function
