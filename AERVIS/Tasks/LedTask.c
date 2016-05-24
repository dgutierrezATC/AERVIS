#include <CoOS.h>			              /*!< CoOS header file	         */
#include "BSP.h"
#include "LedTask.h"
#include "JoyTmr.h"
/*---------------------------- Symbol Define -------------------------------*/
#define STACK_SIZE_LED 256              /*!< Define "taskA" task size */

/*---------------------------- Variable Define -------------------------------*/
OS_STK     led_stk[4][64];	  /*!< Define "taskA" task stack */

void LedTask0(void * parg);
void LedTask1(void * parg);
void LedTask2(void * parg);
void LedTask3(void * parg);

void CreateLedTask(void){
	uint16_t i;
	Init_Leds();
		CoCreateTask (LedTask0, 1 , 3 ,&led_stk[0][63],64);
  		CoCreateTask (LedTask1, 2 , 3 ,&led_stk[1][63],64);
		CoCreateTask (LedTask2, 3 , 3 ,&led_stk[2][63],64);
		CoCreateTask (LedTask3, 4 , 3 ,&led_stk[3][63],64);
}


void LedTask0(void * parg){
		uint8_t nLed,i;
		nLed=parg;
		for (;;) {
		waitForKey(nLed,0);
			LED_Off(0);
			for(i=0;i<10;i++){
				LED_Toggle(0);
				CoTimeDelay(0,0,0,200);
			}

		}

}



void LedTask1(void * parg){
	uint8_t i;
	uint32_t key;
	key=parg;
	for(;;){
		waitForKey(key,0);
		LED_Off(0);
		for(i=1;i<5;i++){
			LED_On(i);
			CoTimeDelay(0,0,0,100);
		}
		for(i=1;i<5;i++){
			LED_Off(5-i);
			CoTimeDelay(0,0,0,100);
		}
		LED_Off(0);
	}
}
void LedTask2(void * parg){
	uint8_t i;
	uint32_t key;
	key=parg;
	for(;;){
		waitForKey(key,0);

		for(i=1;i<5;i++){
			LED_Off(i-1);
			LED_On(i);
			CoTimeDelay(0,0,0,100);
		}

		for(i=1;i<5;i++){
			LED_Off(5-i+1);
			LED_On(5-i);
			CoTimeDelay(0,0,0,100);
		}

		LED_Off(0);
	}
}


void LedTask3(void * parg){

	uint8_t key,i;
	key=parg;
	for (;;) {
		waitForKey(key,0);
			LED_Off(0);
			for(i=1;i<5;i++){
				LED_On(i);
				CoTimeDelay (0,0,0,100);
				LED_Off(i);
				CoTimeDelay (0,0,0,100);
			}
			LED_Off(0);
		}

	}


/*
void animation3(void){
	uint8_t i,j;

	BSP_LED_OFF(0);
	for(i=1;i<5;i++){
		for(j=0;j<10;j++){
			BSP_LED_TOGGLE(i);
			Delay(100);
		}
	}
	BSP_LED_OFF(0);
	for(i=1;i<5;i++){
		for(j=0;j<10;j++){
			BSP_LED_TOGGLE(5-i);
			Delay(100);
		}
	}

}*/
