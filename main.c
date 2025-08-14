/* 
 * File:   main.c
 * Author: Jeffrey
 * Date: July 21, 2025, 9:24 AM
 * Purpose: Use hand motion to turn the head of a model
 * and fade on and off a LED using IR detection
 */

/*
 * motor: total step per rotation = t = 256 * 60
 * 
 * pin layout:
 *  - receiver1 => RA0, RC0
 *  - receiver2 => RA1, RC1
 *  - EYE_LED   => RB6
 *  - motor     => RC3-6
 *  
 */

#include <xc.h>

#define _XTAL_FREQ 31000U //Specify the XTAL crystal FREQ 31kHz

#pragma config WDTE = OFF       // WDT operating mode (WDT Disabled, SWDTEN is ignored)

//    steps per revolution
const int STEP_REV = 2048;

int fade_on = 1; // indicate if LED fades on or off
int adc_rdy = 0; // flag for ADC's Acquisition Time wait time in TMR0
int rev_st_clr;  // flag for first receiver reads 0V
int rev_nd_clr;  // flag for second receiver reads 0V
int adc_cnt = 0;

enum motor_state {
    IDLE,
    CW,
    CCW
};

enum motor_state state;

void TMR0_rst() {
//    period = 209 us
    TMR0H = 0xF3;
    TMR0L = 0xFF;
}

void TMR0_Initialize(){
    PIR0bits.TMR0IF = 0;
	PIE0bits.TMR0IE = 0;

//T0CS LFINTOSC; T0CKPS 1:2; T0ASYNC not_synchronised; 
    T0CON1 = 0x91;
    T0CON0 = 0x90;
}

void __interrupt() isr(void)
{
    if (TMR0IF) {
//        TMR0 takes care of LED fading and ADC delay
        PIR0bits.TMR0IF = 0;
        TMR0_rst();
        
//        ADC
//        cycle between input pins C0 and C1 and measure
//        after waiting for the Acquisition Time passed 
        if (adc_rdy == 0) {
            ADCON0bits.CHS = 0b010000;
        } else if (adc_rdy == 1) {
            ADCON0bits.GOnDONE = 1;
        } else if (adc_rdy == 2) {
            ADCON0bits.CHS = 0b010001;
        } else if (adc_rdy == 3) {
            ADCON0bits.GOnDONE = 1;
        }
        adc_rdy++;
        if (adc_rdy == 4) {
            adc_rdy = 0;
        }
        
//        LED PWM
        if (fade_on == 1) {
            if (PWM3DCH < 0xFF){
                asm("banksel PWM3DCL");
                asm("movlw 40h");
                asm("addwf PWM3DCL,1"); //increment the LSB of PWM3DC
                asm("btfsc STATUS,0");  //if there is a carry from LSB increment
                asm("incf PWM3DCH");    //increment the MSB of PWM3DC
            }
        } else {
            // fade off until duty cycle is 0, then disable timer
            if (PWM3DCH > 0x00 || PWM3DCL > 0x00){
                asm("banksel PWM3DCL");
                asm("movlw 40h");
                asm("subwf PWM3DCL,1"); 
                asm("btfss STATUS,0");  
                asm("decf PWM3DCH");    
            } else {
                PIE0bits.TMR0IE = 0;
            }
        }
    } else if (C1IF) {
//        when motion is detected at the IR receiver
        C1IF = 0;
        ADCON0bits.ADON = 1;
        // fade on LED
        PIE0bits.TMR0IE = 1;    
        fade_on = 1;
        if (CM1CON0bits.OUT == 1) {
            state = CW;
        } else {
            state = CCW;
        }
    } else if (ADIF) {
        ADIF = 0;
    //    IDLE state if less than or equal to 40mV for 
    //    4 consecutive measuring cycles, the cycles are
    //    to make sure that there is no motion and not
    //    just signal misread
        
        if (ADRESH <= 0b00001010) {
            if (adc_rdy == 1) {
                rev_st_clr = 1;
            } else {
                rev_nd_clr = 1;
            }
        } else {
            if (adc_rdy == 1) {
                rev_st_clr = 0;
            } else {
                rev_nd_clr = 0;
            }
        }
        if (rev_st_clr == 1 && rev_nd_clr == 1) {
            adc_cnt++;
        } else {
            adc_cnt = 0;
        }

        if (adc_cnt == 4) {
            adc_cnt = 0;
//            stop measuring and turn off LED
            state = IDLE;
            fade_on = 0;
            ADCON0bits.ADON = 0;
        }
    }
}

void PWM_Initialize() {
    RB6PPS = 0x0B;  //RB6->PWM3:PWM3OUT;

    T2CON = 0x00;
    T2CLKCON = 0x01;
    T2PR = 0xD1;
    T2TMR = 0x00;
    T2CONbits.TMR2ON = 1;
    
    PWM3CON = 0x80;    
    PWM3DCH = 0x00;      
    PWM3DCL = 0x00;
    
    PWM3EN = 1;
}

void COMPARATOR_Initialize() {
    // enable comparator;non-inverting output; hysteresis enabled;async to TMR1
    CM1CON0 = 0b10000010;  
    // posedge and negedge triggered
    CM1CON1 = 0b00000011;
    CM1NCHbits.NCH = 0b000; // C1IN0-   RA1
    CM1PCHbits.PCH = 0b000; // C1IN0+   RA0
    
    // interrupt
    C1IF = 0;
    C1IE = 1;
}

void ADC_Initialize() {
// Configure the ADC module:
    ADCON1bits.ADCS = 0b011;
    ADCON1bits.ADPREF = 0b11;
    FVRCONbits.FVREN = 1;
    FVRCONbits.ADFVR = 0b10;    //FVR = 2.048V
    ADACT = 0x02;   // trigger by TMR0
    ADFM = 0;

//    interrupt
    PIR1bits.ADIF = 0;
    PIE1bits.ADIE = 1;
}

void main(void) {
//    pins initialization
    ANSELA = 0;
    ANSELB = 0;
    ANSELC = 0x03;
    TRISA = 0x03;
    TRISB = 0x00;
    TRISC = 0x03;

//  peripherals initialization
    PWM_Initialize();
    TMR0_Initialize();
    COMPARATOR_Initialize();
    ADC_Initialize();
    
    GIE = 1;
    PEIE = 1;
    
    PORTC = 0x08;
        
    state = IDLE;

    while (1) {
        if (state == CW) {
            asm("banksel PORTC");
            asm("lslf PORTC, 1");
            asm("btfss PORTC,7");
            asm("bra 2");
            asm("movlw 08h");
            asm("movwf PORTC");
            __delay_ms(1000);
        } else if (state == CCW) {
            asm("banksel PORTC");
            asm("lsrf PORTC, 1");
            asm("btfss PORTC,2");
            asm("bra 2");
            asm("movlw 40h");
            asm("movwf PORTC");
            __delay_ms(1000);
        }
    }
    
    return;
}
