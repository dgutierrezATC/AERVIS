#include "AERdriver.h"

/* Pines del bus AER */

#define RCC_AHB1Periph_GPIOx_BUS RCC_AHB1Periph_GPIOA

#define AERbus_0 GPIO_Pin_0
#define AERbus_1 GPIO_Pin_1
#define AERbus_2 GPIO_Pin_2
#define AERbus_3 GPIO_Pin_3
#define AERbus_4 GPIO_Pin_4
#define AERbus_5 GPIO_Pin_5
#define AERbus_6 GPIO_Pin_6
#define AERbus_7 GPIO_Pin_7
#define AERbus_8 GPIO_Pin_8
#define AERbus_9 GPIO_Pin_9
#define AERbus_10 GPIO_Pin_10
#define AERbus_11 GPIO_Pin_11
#define AERbus_12 GPIO_Pin_12
#define AERbus_13 GPIO_Pin_13
#define AERbus_14 GPIO_Pin_14
#define AERbus_15 GPIO_Pin_15
#define AERbus GPIO_Pin_All

#define AERbus_port GPIOA

/* Pines del protocolo */

#define RCC_AHB1Periph_GPIOx_PROT RCC_AHB1Periph_GPIOG

#define AERreq GPIO_Pin_11
#define AERack GPIO_Pin_14

#define AERprot_port GPIOG

/* Timer para el timeStamp */

#define RCC_APB1Periph_TIMx RCC_APB1Periph_TIM14
#define AERtimer TIM14


/* Funcion de inicializacion de los pines del bus AER */

void Init_AERbus(void){

	//1- Estructura de Configuración
	GPIO_InitTypeDef gpio;

	//2 - Habilitación del reloj del periférico
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOx_BUS, ENABLE);

	//3 - Relleno de la estructura de configuración
	gpio.GPIO_Pin = AERbus;	//La configuración afecta a los pines 2 y 3
	gpio.GPIO_Mode = GPIO_Mode_IN;				//Pines como salidas
	//gpio.GPIO_Speed = GPIO_Speed_50MHz;			//Velocidad del puerto a 100MHz
	//gpio.GPIO_OType = GPIO_OType_PP;			//La salida es en modo PushPull
	gpio.GPIO_PuPd = GPIO_PuPd_NOPULL;			//Sin resistencas pull-up ni pull-down

	//4 - Escritura de la configuración en el periférico
	GPIO_Init(AERbus_port,&gpio);	//Se especifica el periférico y un puntero la estructura de configuración
}

/* Funcion de inicializacion de los pines de protocolo AER */

void Init_AERprot(void){

	GPIO_InitTypeDef gpio;

	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOx_PROT, ENABLE);

	gpio.GPIO_Pin = AERreq;
	gpio.GPIO_Mode = GPIO_Mode_IN;
	//gpio.GPIO_Speed = GPIO_Speed_50MHz;
	//gpio.GPIO_OType = GPIO_OType_PP;
	gpio.GPIO_PuPd = GPIO_PuPd_NOPULL;

	GPIO_Init(AERprot_port,&gpio);

	gpio.GPIO_Pin = AERack;
	gpio.GPIO_Mode = GPIO_Mode_OUT;
	gpio.GPIO_Speed = GPIO_Speed_50MHz;
	gpio.GPIO_OType = GPIO_OType_PP;
	gpio.GPIO_PuPd = GPIO_PuPd_NOPULL;

	GPIO_Init(AERprot_port,&gpio);
}

void Init_AERtimestamp(void){

	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIMx , ENABLE);

	/* -----------------------------------------------------------------------
	TIM14 Configuration: Output Compare Timing Mode:
	*/
	//  TIM_TimeBaseStructure.TIM_Prescaler = 83;
	TIM_TimeBaseStructure.TIM_Prescaler = 41;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseStructure.TIM_Period = 65535;
	TIM_TimeBaseStructure.TIM_ClockDivision = 0;
	TIM_TimeBaseStructure.TIM_RepetitionCounter = 0;

	TIM_TimeBaseInit(AERtimer, &TIM_TimeBaseStructure);

	/* TIM14 enable counter */
	TIM_Cmd(AERtimer, ENABLE);
}

void Init_AER(void){
	Init_AERbus();
	Init_AERprot();
	Init_AERtimestamp();
}

uint8_t AER_getRequest(void){
	return GPIO_ReadInputDataBit(AERprot_port, AERreq);
}

void AER_upACK(void){
	GPIO_SetBits(AERprot_port, AERack);
	//GPIO_WriteBit(AERprot_port, AERack, Bit_SET);
}

void AER_downACK(void){
	GPIO_ResetBits(AERprot_port, AERack);
	//GPIO_WriteBit(AERprot_port, AERack, Bit_RESET);
}

void AER_readBus(uint16_t *aer_data){
	uint16_t data = 0;

	data = GPIO_ReadInputData(AERbus_port);
	*aer_data = data;
}

void AER_AddToBuff(uint16_t **aer_dataBuff, uint8_t pos, uint16_t aer_data, uint16_t aer_dataTimeStamp){

	aer_dataBuff[0][pos] = aer_data;
	aer_dataBuff[1][pos] = aer_dataTimeStamp;
}

//void AER_readBus(uint16_t **aer_data, uint8_t pos){
//	uint16_t data = 0;
//
//	data = GPIO_ReadInputData(AERbus_port);
//	aer_data[0][pos] = data;
//}

uint32_t AER_getTimeStamp(void){
	return TIM_GetCounter(AERtimer);
}

