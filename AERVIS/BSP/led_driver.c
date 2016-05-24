#include "stm32f4xx_conf.h"
#include "led_driver.h"

void Init_Leds(void){

	//1- Estructura de Configuración
	GPIO_InitTypeDef gpio;

	//2 - Habilitación del reloj del periférico
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOH, ENABLE);

	//3 - Relleno de la estructura de configuración
	gpio.GPIO_Pin = GPIO_Pin_2 | GPIO_Pin_3;	//La configuración afecta a los pines 2 y 3
	gpio.GPIO_Mode = GPIO_Mode_OUT;				//Pines como salidas
	gpio.GPIO_Speed = GPIO_Speed_50MHz;			//Velocidad del puerto a 100MHz
	gpio.GPIO_OType = GPIO_OType_PP;			//La salida es en modo PushPull
	gpio.GPIO_PuPd = GPIO_PuPd_NOPULL;			//Sin resistencas pull-up ni pull-down

	//4 - Escritura de la configuración en el periférico
	GPIO_Init(GPIOH,&gpio);	//Se especifica el periférico y un puntero la estructura de configuración

	//Inicializar LED3 y LED4, en PI8 y PI10 respectivamente

	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOI, ENABLE);

	gpio.GPIO_Pin = GPIO_Pin_8 | GPIO_Pin_10;	//La configuración afecta a los pines 8 y 10
	gpio.GPIO_Mode = GPIO_Mode_OUT;				//Pines como salidas
	gpio.GPIO_OType = GPIO_OType_PP;			//La salida es en modo PushPull
	gpio.GPIO_PuPd = GPIO_PuPd_NOPULL;			//Sin resistencas pull-up ni pull-down

	GPIO_Init(GPIOI,&gpio);		//Se especifica el periférico y un puntero  la estructura de configuración


}

void LED_On (uint8_t led){
	switch (led){
	case 0:									//LEDS 1..4
		GPIO_SetBits(GPIOH,GPIO_Pin_2 | GPIO_Pin_3);
		GPIO_SetBits(GPIOI,GPIO_Pin_8 | GPIO_Pin_10);
		break;
	case 1:									//LED 1
		GPIO_SetBits(GPIOH,GPIO_Pin_2);
		break;
	case 2:									//LED 2
		GPIO_SetBits(GPIOH,GPIO_Pin_3);
		break;
	case 3:									//LED 3
		GPIO_SetBits(GPIOI,GPIO_Pin_8);
		break;
	case 4:									//LED 4
		GPIO_SetBits(GPIOI, GPIO_Pin_10);
		break;
	}
}


void LED_Off (uint8_t led){
	switch (led){
	case 0:
		GPIO_ResetBits(GPIOH,GPIO_Pin_2 | GPIO_Pin_3);
		GPIO_ResetBits(GPIOI,GPIO_Pin_8 | GPIO_Pin_10);
		break;
	case 1:
		GPIO_ResetBits(GPIOH,GPIO_Pin_2);
		break;
	case 2:
		GPIO_ResetBits(GPIOH,GPIO_Pin_3);
		break;
	case 3:
		GPIO_ResetBits(GPIOI,GPIO_Pin_8);
		break;
	case 4:
		GPIO_ResetBits(GPIOI, GPIO_Pin_10);
		break;
	}
}

void LED_Toggle(uint8_t led){

	switch (led){
	case 0:
		GPIO_ToggleBits(GPIOH, GPIO_Pin_2 | GPIO_Pin_3);
		GPIO_ToggleBits(GPIOI, GPIO_Pin_8 | GPIO_Pin_10);
		break;
	case 1:
		GPIO_ToggleBits(GPIOH,GPIO_Pin_2);
		break;
	case 2:
		GPIO_ToggleBits(GPIOH,GPIO_Pin_3);
		break;
	case 3:
		GPIO_ToggleBits(GPIOI,GPIO_Pin_8);
		break;
	case 4:
		GPIO_ToggleBits(GPIOI,GPIO_Pin_10);
		break;
	}

}


