/**
 * Name:        Joshua Foster
 * Date:        7/31/23
 * Assignment:  Lab 4 - Ultrasonic Alarm
 * Youtube:     https://youtu.be/3_YBsZ8sG6w
 *
 * This program:
 * Uses a Ultrasonic Range Finder to judge if something is in front of the Range Finder
 * based on how far the signal travels (determined by the amount of cycles between the
 * signal being sent and received). If this distance is changed drastically then the
 * buzzer (alarm) sounds until reset via button push.
 */

#include <msp430.h>
#include <stdbool.h>
#include <math.h>

#define TOLERANCE .023

typedef enum {IDLE, ARMING, ARMED} state_t;

float compareDis = 0; // global compare value

void init();
void initGPIO();
void initTimers();
void initUltrasonic();
float getDistance();
bool isButtonPressed();
state_t runIdle();
state_t runArming();
state_t runArmed();
void reset();


int main(void)
{
    state_t currentState = IDLE;
    state_t nextState = currentState;
	init();
	while(1){
	    switch(currentState){
            case IDLE:
                nextState = runIdle();
                break;
            case ARMING:
                nextState = runArming();
                break;
            case ARMED:
                nextState = runArmed();
                break;
	    }
	    currentState = nextState;
	}

}

void init(){
    WDTCTL = 0x5A80; // Stop watchdog timer
    PM5CTL0 = 0xFFFE; // Disable the GPIO power-on default high-impedance mode

    initGPIO();
    initTimers();

}

void initGPIO(){
    //button config
    P2DIR = 0x00; // input
    P2REN = BIT3; // enable pull-up resistor on P2.3
    P2OUT = BIT3; // setting P2.3 as high to configure pull-up

    //buzzer config
    P6DIR |= BIT3;
    P6OUT &= ~BIT3;
}

void initTimers(){
    // Configure Timer B0 to use SMCLK and be stopped
    TB0CTL = TBSSEL__SMCLK | MC__STOP;
}

void initUltrasonic(){
    // setting it to low initially
    P6DIR |= BIT1;
    P6OUT &= ~BIT1;
    __delay_cycles(10);
    // setting it to high
    P6OUT |= BIT1;
    __delay_cycles(10);
    P6OUT &= ~BIT1; //setting it back low

    P6DIR &= ~BIT1;

}

float getDistance(){
    //resetting counts and counts register
    float counts = 0;
    TB0R = 0;
    initUltrasonic();

    //wait for signal to go high
    while((P6IN & BIT1) == 0x00);
    TB0CTL = TBSSEL__SMCLK | MC__CONTINUOUS;
    //wait for signal to go low
    while((P6IN & BIT1));
    TB0CTL = TBSSEL__SMCLK | MC__STOP;
    counts = TB0R; //store value in Timer B's count register into counts variable
    //calculations
    return ((counts/1000000.0) * 343)/2; // counts / 1000000(1mHz) for seconds * 343 (speed of sound) for meters / 2 for only one of the travel distances (to/from)
}

bool isButtonPressed(){
    if((P2IN & BIT3) == 0x00){
        // debounce
        while((P2IN & BIT3) == 0x00){
            __delay_cycles(500);
        }
        return true;
    }
    return false;
}

/**
 * Idle state => stays until button is pressed
 * @return state_t ARMING once button is pressed
 */
state_t runIdle(){
    while(!isButtonPressed());

    return ARMING;
}

/**
 * Arming state => runs getDistance() once then stores it in global compareDis variable for later
 * @return state_t ARMED
 */
state_t runArming(){
    compareDis = getDistance();

    return ARMED;
}

/**
 * Armed state => constantly checks distance signal is traveling while the button is not being pressed and stores it in local variable
 * compares local variable to global benchmark compareDis variable. If difference in value exceeds tolerance, sound buzzer.
 *
 * Once button is pressed, reset() is called and return IDLE
 *
 * @return state_t IDLE
 */
state_t runArmed(){
    float newDistance = 0.0;

    do {
        newDistance = getDistance();
        if(fabs(newDistance - compareDis) > TOLERANCE){
            P6OUT |= BIT3; //turn on buzzer
        }
    } while(!isButtonPressed());
    reset();
    return IDLE;
}


/*
 * Helps to reset the program. Primarily turns off the buzzer
 */
void reset(){
    //turn off buzzer
    P6OUT &= ~BIT3;
}
