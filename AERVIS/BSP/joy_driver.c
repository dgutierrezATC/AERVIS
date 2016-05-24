#include "stm32f4xx_conf.h"
#include "joy_driver.h"

void Init_Joy(void){

	//1- Estructura de Configuración
	GPIO_InitTypeDef gpio;

	//2 - Habilitación del reloj del periférico
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);

	//3 - Relleno de la estructura de configuración
	gpio.GPIO_Pin = GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_6;	//La configuración afecta a los pines 2, 3, 4 , 5 y 6
	gpio.GPIO_Mode = GPIO_Mode_IN;				//Pines como entradas
	gpio.GPIO_PuPd = GPIO_PuPd_UP;				//Con resistencia de pull-up

	//4 - Escritura de la configuración en el periférico
	GPIO_Init(GPIOE,&gpio);		//Se especifica el periférico y un puntero  la estructura de configuración

}

uint8_t Read_Joy (void){

	uint8_t joy=0;

	if(GPIO_ReadInputDataBit(GPIOE, GPIO_Pin_2)==0){
		joy=1;	//A pulsada
	}
	if(GPIO_ReadInputDataBit(GPIOE, GPIO_Pin_3)==0){
		joy=2;	//B pulsada
	}
	if(GPIO_ReadInputDataBit(GPIOE, GPIO_Pin_4)==0){
		joy=3;	//C pulsada
	}
	if(GPIO_ReadInputDataBit(GPIOE, GPIO_Pin_5)==0){
		joy=4;	//D pulsada
	}
	if(GPIO_ReadInputDataBit(GPIOE, GPIO_Pin_6)==0){
		joy=5;	//D pulsada
	}

	return joy;



}
