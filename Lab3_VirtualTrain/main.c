/**
 * Name:        Joshua Foster
 * Date:        7/6/2023
 * Assignment:  Lab 3 Virtual Train
 * Youtube:     https://youtu.be/qThGnvyonN0
 *
 * This program:
 * When this code is compiled and begins to run the train is displayed on the 4-digit display in the default position, stopped
 * The user can then click S1 to begin moving (default forward) or S2 to change the direction the train will move, regardless
 * of if it is in motion or not
 */

#include <msp430.h> 
#include <stdint.h>
#include <stdbool.h>
#include "Four_Digit_Display.h"

#define LED_DELAY_MS 1000000 // delay for 1 second
#define RED_DIR     P1DIR
#define RED_OUT     P1OUT
#define RED_PIN     BIT0
#define GREEN_DIR   P6DIR
#define GREEN_OUT   P6OUT
#define GREEN_PIN   BIT6

int count = 0;

typedef enum {STOPPED, FORWARD, REVERSE, MAX_STATES} state_t;

typedef struct{
    bool s1Pressed;
    bool s2Pressed;
    bool forward;
    bool moving;
} inputs_t;

void init();
state_t runStopped(inputs_t* inputs);
state_t runForward(inputs_t* inputs);
state_t runReverse(inputs_t* inputs);
void setLED(state_t state);
bool isS1Pressed(inputs_t* inputs);
bool isS2Pressed(inputs_t* inputs);
void setDisplay();

int main(void)
{
    state_t currentState = STOPPED;
    state_t nextState = currentState;
    inputs_t inputs;
    inputs.s1Pressed = false;
    inputs.s2Pressed = false;
    inputs.forward = true;
    inputs.moving = false;
    init();
    four_digit_init();
    four_digit_set_brightness(BRIGHT_TYPICAL);
    four_digit_set_point(POINT_ON);

    while(1){
        switch(currentState){
        case STOPPED:
            setLED(currentState);
            nextState = runStopped(&inputs); // red led
            break;
        case FORWARD:
            setLED(currentState);
            nextState = runForward(&inputs); // green led
            break;
        case REVERSE:
            setLED(currentState);
            nextState = runReverse(&inputs); // green led
            break;

        }
        currentState = nextState;
    }
}


void init(){
    WDTCTL = 0x5A80; // Stop watchdog timer
    PM5CTL0 = 0xFFFE; // Disable the GPIO power-on default high-impedance mode

    //button config
    //s1 = moving or stopped
    P4DIR = 0x00; // input
    P4REN = BIT1; // enable pull-up resistor on P4.1
    P4OUT = BIT1; // setting P4.1 as high to configure pull-up
    //s2 = direction
    P2DIR = 0x00; // input
    P2REN = BIT3; // enable pull-up resistor on P2.3
    P2OUT = BIT3; // setting P2.3 as high to configure pull-up

    //LED config
    RED_DIR = RED_PIN;
    GREEN_DIR = GREEN_PIN;

}

/**
 * The run function for the stopped state
 */
state_t runStopped(inputs_t* inputs){
    setDisplay();
    while(1){
        isS2Pressed(inputs);
        if(isS1Pressed(inputs)){
            if(inputs->moving == false){
                if(inputs->forward == true){
                    return FORWARD;
                } else if(inputs->forward == false){
                    return REVERSE;
                }
            }
        }
    }


}

/**
 * The run function for the forward state
 */
state_t runForward(inputs_t* inputs){
    while(1){
        if(count > 11){
            count = 0;
        }
        setDisplay();
        count++;
        if(isS1Pressed(inputs)){
            return STOPPED;
        } else if(isS2Pressed(inputs)){
            return REVERSE;
        }
        __delay_cycles(LED_DELAY_MS);
    }
}

/**
 * The run function for the reverse state
 */
state_t runReverse(inputs_t* inputs){
    while(1){
        if(count < 0){
            count = 11;
        }
        setDisplay();
        count--;
        if(isS1Pressed(inputs)){
            return STOPPED;
        } else if(isS2Pressed(inputs)){
            return FORWARD;
        }
        __delay_cycles(LED_DELAY_MS);
    }

}

/**
 * Sets green or red LED based on the passed in state
 */
void setLED(state_t state){
    if(state == STOPPED){                           // not moving = red
        GREEN_OUT &= ~ GREEN_PIN;
        RED_OUT |= RED_PIN;
    } else {                                    // moving = green
        RED_OUT &= ~RED_PIN;
        GREEN_OUT |= GREEN_PIN;

    }
}

/**
 * Changes whether the train is moving or not based on S1 being pressed
 */
bool isS1Pressed(inputs_t* inputs){
    if((P4IN & BIT1) == 0x00){
        while((P4IN & BIT1) == 0x00){
            __delay_cycles(500);
        }
        inputs->s1Pressed = !(inputs->s1Pressed);
        inputs->moving = !(inputs->moving);
        return true;
    }
    return false;
}

/**
 * Changes value of forward (direction) based on S2 being pressed
 */
bool isS2Pressed(inputs_t* inputs){
    if((P2IN & BIT3) == 0x00){
        while((P2IN & BIT3) == 0x00){
            __delay_cycles(500);
        }
        inputs->s2Pressed = !(inputs->s2Pressed);
        inputs->forward = !(inputs->forward);
        return true;
    }
    return false;
}

/**
 * switch statement based on global count variable -> displays the train segment on the board corresponding to the count value
 */
void setDisplay(){
    switch(count){
    case 0:
        display_segment(POS_2, 0x00);
        display_segment(POS_3, 0x00);
        display_segment(POS_4, 0x00);
        display_segment(POS_1, SEG_F | SEG_E | SEG_D);
        break;
    case 1:
        display_segment(POS_2, 0x00);
        display_segment(POS_3, 0x00);
        display_segment(POS_4, 0x00);
        display_segment(POS_1, SEG_A | SEG_F | SEG_E);
        break;
    case 2:
        display_segment(POS_3, 0x00);
        display_segment(POS_4, 0x00);
        display_segment(POS_2, SEG_A);
        display_segment(POS_1, SEG_A | SEG_F);
        break;
    case 3:
        display_segment(POS_4, 0x00);
        display_segment(POS_3, SEG_A);
        display_segment(POS_2, SEG_A);
        display_segment(POS_1, SEG_A);
        break;
    case 4:
        display_segment(POS_1, 0x00);
        display_segment(POS_4, SEG_A);
        display_segment(POS_3, SEG_A);
        display_segment(POS_2, SEG_A);
        break;
    case 5:
        display_segment(POS_1, 0x00);
        display_segment(POS_2, 0x00);
        display_segment(POS_4, SEG_B | SEG_A);
        display_segment(POS_3, SEG_A);
        break;
    case 6:
        display_segment(POS_1, 0x00);
        display_segment(POS_2, 0x00);
        display_segment(POS_3, 0x00);
        display_segment(POS_4, SEG_C | SEG_B | SEG_A);
        break;
    case 7:
        display_segment(POS_1, 0x00);
        display_segment(POS_2, 0x00);
        display_segment(POS_3, 0x00);
        display_segment(POS_4, 0x00);
        display_segment(POS_4, SEG_D | SEG_C | SEG_B);
        break;
    case 8:
        display_segment(POS_1, 0x00);
        display_segment(POS_2, 0x00);
        display_segment(POS_3, SEG_D);
        display_segment(POS_4, SEG_D | SEG_C);
        break;
    case 9:
        display_segment(POS_1, 0x00);
        display_segment(POS_2, SEG_D);
        display_segment(POS_3, SEG_D);
        display_segment(POS_4, SEG_D);
        break;
    case 10:
        display_segment(POS_4, 0x00);
        display_segment(POS_1, SEG_D);
        display_segment(POS_2, SEG_D);
        display_segment(POS_3, SEG_D);
        break;
    case 11:
        display_segment(POS_3, 0x00);
        display_segment(POS_4, 0x00);
        display_segment(POS_1, SEG_E | SEG_D);
        display_segment(POS_2, SEG_D);
        break;
    }
}
