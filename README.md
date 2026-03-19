# User tracker using IR sensors
This project contains the software and design to build a device that will rotate in respond to user's hand movement, such that it follows or avoid user's hand. This is used in a hobby project of my to add on to a robot model.

## Components
IR transmitters and receivers will be used to detect user's hand movement, a motor will be used such that the device follows the user's hand. A LED will be used to simulate the eye of the device. And PIC16F15344 will be used to control the signals.

- 1 x PIC16F15344
- 1 x 28BYJ-48 Stepper Motor
- 1 x yellow LED
- 2 x IR receivers
- 2 x IR transmitters
- 3 x 1 kOhm current-limiting resistor
- 2 x 10 kOhm resistor

## Design
A pair of IR transmitter/receiver will be used to create a sensor to detect how close user's hand is to the device.
Two such sensors will be used to compare the distance of the sensors and user's hand, and device will be mounted on a motor to rotate in the direction correpsonding to user's proxmity with the sensors. 

https://github.com/user-attachments/assets/98550bba-aff8-4348-be6f-40275e6825a6

## Configuration
The circuit and PCB will need to be redesigned to add features or change to fit your own project, but if you wish to just try out the design, the PCB can be glued to the motor and it will function as intended.
You may want to change the sensitivity of the device, the steps are as follow:
1. determine locations of the 'rec_1' and 'rec_2' points on your own circuit ('rec_1' and 'rec_2' will be shown at the schematic below)
2. put your hand at the distance for the desired movement detection range and measure the voltage at the 'rec_1' and 'rec_2' points
3. pick the greater voltage value (in mV) from the two points, divide it by 2 and convert it to a binary number, round up the 2 least significant bits and logical right shift by 2
4. change the ADCRESH_REG variable at line 27 at the [main.c](main.c) file to the value from step 3

For example, in my device I measured 363mV 
363mV -> 181.5 -> 0b1011_0101 -> 0b1011_1000 -> 0b0010_1110
And '0b00101110' is my value for ADCRESH_REG


## Schematic
![circuit_schematic](https://github.com/user-attachments/assets/225a1211-14ed-4425-a27e-698bf8afacdc)
![circuit_perfboard](https://github.com/user-attachments/assets/92b2c162-c754-4ff7-a7a7-1b119258b3c6)


## Microcontroller requirements
A different microcontroller can be used, but it have to support the following features/peripherals
- timer
- comparator
- PWM (uses a second timer in this example)
- ADC

## How it works
The device has 3 states: IDLE, CW, CCW  
In IDLE state, motor will stop, LED will fade off if it was on(TMR0 will slowly decrement the duty cycle of the PWM). Comparator will be used to compare voltage difference between IR receivers, the change in output will turn on the LED to start fading on(TMR0 will slowly increment the duty cycle of the PWM), and device will enter CW(clockwise) state or CCW(counter-clockwise) state depends on the comparator output.  
In CW state, motor will turn clockwise, and ADC will continuously detect IR receivers' voltage level. Once both receivers' voltage level is lower than specific level for a few cycles (to avoid misread), device will enter IDLE state.  
In CCW state, motor will turn counter-clockwise, all other operations are the same as CW state.

## Possible issues and improvements
- A different sensor was originally used, as the current detection method has very limited range. The first iteration used PIR sensor, but it cannot be used on moving devices and has a ~2 sec delay time after motions stopped so it wasn't used. Upon further research time of flight sensor may be used, but it is more costly and complex.
- ADC voltage level at which device switches back to IDLE state (line 27) may need to be changed, this value will be dependent on the applications as the sensors will return different voltage under different environment.
- To save power in IDLE mode, a sleep function can be implemented
- To save power in IDLE mode, IR transmitter can be turn on for a short interval after a small delay (to have a PWM on the IR transmitter)
