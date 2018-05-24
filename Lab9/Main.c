// ADCTestmain.c
// Runs on LM4F120/TM4C123
// Provide a function that initializes Timer0A to trigger ADC
// SS3 conversions and request an interrupt when the conversion
// is complete.
// Daniel Valvano
// May 3, 2015

/* This example accompanies the book
   "Embedded Systems: Real Time Interfacing to Arm Cortex M Microcontrollers",
   ISBN: 978-1463590154, Jonathan Valvano, copyright (c) 2015

 Copyright 2015 by Jonathan W. Valvano, valvano@mail.utexas.edu
    You may use, edit, run or distribute this file
    as long as the above copyright notice remains
 THIS SOFTWARE IS PROVIDED "AS IS".  NO WARRANTIES, WHETHER EXPRESS, IMPLIED
 OR STATUTORY, INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF
 MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE.
 VALVANO SHALL NOT, IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL,
 OR CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.
 For more information about my classes, my research, and my books, see
 http://users.ece.utexas.edu/~valvano/
 */

// center of X-ohm potentiometer connected to PE3/AIN0
// bottom of X-ohm potentiometer connected to ground
// top of X-ohm potentiometer connected to +3.3V through X/10-ohm ohm resistor
#include <stdint.h>
#include "../inc/tm4c123gh6pm.h"
#include "ADCT0ATrigger.h"
#include "PLL.h"
#include "UART.h"
#include "ST7735.h"
#include "calib.h"

void DisableInterrupts(void); // Disable interrupts
void EnableInterrupts(void);  // Enable interrupts
long StartCritical (void);    // previous I bit, disable interrupts
void EndCritical(long sr);    // restore I bit to previous value
void WaitForInterrupt(void);  // low power mode

//debug code
//
// This program periodically samples ADC0 channel 0 and stores the
// result to a global variable that can be accessed with the JTAG
// debugger and viewed with the variable watch feature.


extern uint32_t data[100];
extern int bufferIndex;

void startScreen() {
	ST7735_SetCursor(0,0); ST7735_OutString("Temperature Lab");
	ST7735_PlotClear(1000,4000); // range from 0 to 4095
	ST7735_SetCursor(0,1); ST7735_OutString("N=");
	ST7735_SetCursor(0,2); ST7735_OutString("T="); 
	ST7735_OutString("     ");
	ST7735_OutString(" C");
}

int j = 0;
int N = 100;
int fs = 20;

void draw(int data) {
	int Temperature = getTemp(data);
	ST7735_PlotPoint(Temperature); // Measured temperature
	if((j&(N-1))==0){ // fs sampling, fs/N samples plotted per second
		ST7735_PlotNextErase(); // overwrites N points on same line
	}
	if((j%fs)==0) { // fs sampling, 1 Hz display of numerical data
		ST7735_SetCursor(3,1); 	
		ST7735_OutString("     ");
		ST7735_SetCursor(3,1); 	
		ST7735_OutUDec(data); // 0 to 4095
		int outTemp = Temperature/100;
		ST7735_SetCursor(3,2);
 		ST7735_OutString("     ");
		ST7735_SetCursor(3,2);
		ST7735_OutUDec(outTemp); // 0.01 C
		ST7735_OutString(".");
		ST7735_OutUDec(Temperature%100);
	}
	j++;
}

int main(void){
  PLL_Init(Bus80MHz);                      // 80 MHz system clock
	UART_Init();              // initialize UART device
	ST7735_InitR(INITR_REDTAB);
	startScreen();

  SYSCTL_RCGCGPIO_R |= 0x00000020;         // activate port F
  ADC0_InitTimer0ATriggerSeq3(0, 80000); // ADC channel 0, 1000 Hz sampling
  GPIO_PORTF_DIR_R |= 0x04;                // make PF2 out (built-in LED)
  GPIO_PORTF_AFSEL_R &= ~0x04;             // disable alt funct on PF2
  GPIO_PORTF_DEN_R |= 0x04;                // enable digital I/O on PF2
                                           // configure PF2 as GPIO
  GPIO_PORTF_PCTL_R = (GPIO_PORTF_PCTL_R&0xFFFFF0FF)+0x00000000;
  GPIO_PORTF_AMSEL_R = 0;                  // disable analog functionality on PF
  GPIO_PORTF_DATA_R &= ~0x04;              // turn off LED
  EnableInterrupts();
	
  while(1){
		//GPIO_PORTF_DATA_R ^= 0x04;             // toggle LED
		if(bufferIndex >= 100) {
			for(int a = 0; a < 100; a++) {
				draw(data[a]);
			}
			bufferIndex = 0;
		}
	}
	/*
	* Uncomment to send 100 samples to UART
	*/
	/*Disable_ADC();
	UART_OutString("\n\rADC data =");
	for (int i = 0; i < 100; i++) {
		UART_OutUDec(data[i]);
		UART_OutString("\n\r");
	}
	bufferIndex = 0;*/
}

