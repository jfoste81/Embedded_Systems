/**
 * Name:        Joshua Foster
 * Date:        8/8/2023
 * Assignment:  Lab 5 Game Controller
 * Youtube:     https://youtu.be/4Y2f_xe_Rb0
 *
 * This program:
 * When compiled and ran, the code with initialize all of the necessary components and then will begin reading from the ADC Conversion Memory Register and awaiting button presses via polling and interrupts.
 * While this is going on, it will also be transmitting analog signals that have been converted to digital signals via the UART Transmit Buffer Register to the python game.
 * The user should be able to launch game.py after this code has been launched and move the plane across the x-axis via the potentiometer and fire missles at enemy planes with the S1 and S2 buttons.
 */

#include <msp430.h>
#include <stdbool.h>
#include <stdint.h>

#define BUTTON_DELAY    150

void init();
void initGPIO();
void initClocks();
void initUART();
void initADC();
bool isS1Pressed();
bool isS2Pressed();
uint8_t readADC();
void transmit(uint8_t data);

int main(void)
{
    init();

    uint8_t data = 0;

    while(1){

        data = readADC();

        if (isS1Pressed() || isS2Pressed()) {
            transmit(255);
        }
        if(data != 255) {
            transmit(data);
        }
    }
}

void init(){
    WDTCTL = 0x5A80; // Stop watchdog timer
    PM5CTL0 = 0xFFFE; // Disable the GPIO power-on default high-impedance mode

    __enable_interrupt();

    initGPIO();
    initClocks();
    initUART();
    initADC();
}

void initGPIO(){
    // Configure potentiometer
    P1DIR &= ~BIT1;
    P1IN |= ~BIT1;

    // Configure S1
    P4DIR &= ~BIT1;
    P4REN |= BIT1;
    P4OUT |= BIT1;
    P4IE |= BIT1;
    P4IES |= BIT1;
    P4IFG &= ~BIT1;

    // Configure S2
    P2DIR &= ~BIT3;
    P2REN |= BIT3;
    P2OUT |= BIT3;
    P2IE |= BIT3;
    P2IES |= BIT3;
    P2IFG &= ~BIT3;
}


void initClocks(){
    // Set XT1CLK as ACLK source
    CSCTL4 |= SELA__XT1CLK;
    // Use external clock in low-frequency mode
    CSCTL6 |= XT1BYPASS_1 | XTS_0;
}

void initUART(){
    // Configure UART pins
    P4SEL0 |= BIT3 | BIT2; // set 2-UART pin as second function
    P4SEL1 &= ~(BIT3 | BIT2);

    P1SEL0 |= BIT1;       // set UART pin as second function
    P1SEL1 &= ~BIT1;

    // Configure UART
    UCA1CTLW0 = UCSWRST; // Hold UART in reset state
    UCA1CTLW0 |= UCSSEL__ACLK;

    UCA1BR0 = 3; // 32768/9600 from Baud Rate Calc
    UCA1BR1 = 0;
    UCA1MCTLW |= 0x9200;
    UCA1CTLW0 &= ~UCSWRST; // Release reset state for operation
    UCA1IE |= UCRXIE; // Enable USCI_A0 RX interrupt
}

void initADC(){
    // Configure ADC
    ADCCTL0 |= ADCSHT_2 | ADCON;        // ADCON S&H=16 ADC clks
    ADCCTL1 |= ADCSHP;                  // ADCCLK = MODOSC; sampling timer
    ADCCTL2 &= ~ADCRES;
    ADCCTL2 |= ADCRES;                // 8-bit conversion results
    ADCMCTL0 |= ADCINCH_1 | ADCSREF_0;  // Internal channel 1 select; vref=3.3v
}

/**
 * @return bool value where true if s1 is pressed, false otherwise
 */
bool isS1Pressed() {
    return (P4IN & BIT1) == 0x00;
}

/**
 * @return bool value where true if s2 is pressed, false otherwise
 */
bool isS2Pressed(){
    return (P2IN & BIT3) == 0x00;
}

/**
 * Enable and start ADC conversion
 *
 * @return int ADC memory register value that has been converted
 */
uint8_t readADC(){
    ADCCTL0 |= ADCENC | ADCSC;  // Sampling and conversion start

    while(ADCCTL1 & ADCBUSY);   // Wait for conversion to finish
    return ADCMEM0;
}

/**
 * Transmits data via UART transmit buffer
 * @param data 8-bit integer value that will be transmitted via buffer
 */
void transmit(uint8_t data) {
    // Configure UART1 to transmit data
    while (!(UCA1IFG & UCTXIFG));   // Wait for UART transmit buffer to be empty

    UCA1TXBUF = data;   // Send data over UART

}

#pragma vector=USCI_A1_VECTOR
__interrupt void USCI_A1_ISR(void)
{
    UCA1TXBUF = UCA1RXBUF;
    __no_operation();
    // Clear flag
    UCA1IFG = 0x00;
}

/**
 * Interrupt for S1
 */
#pragma vector=PORT4_VECTOR
__interrupt void Port_4ISR(){
    __delay_cycles(BUTTON_DELAY);

    transmit(255);

    P4IFG &= ~BIT1;
}

/**
 * Interrupt for S2
 */
#pragma vector=PORT2_VECTOR
__interrupt void Port_2ISR(){
    __delay_cycles(BUTTON_DELAY);

    transmit(255);

    P2IFG &= ~BIT3;
}
