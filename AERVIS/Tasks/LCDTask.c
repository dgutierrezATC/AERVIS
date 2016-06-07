#include "All.h"

//Parametros del Histograma
#define HISTOGRAM_FRAME(x) x*1000 //en ms

#define NUM_CHANNELS 64

uint8_t isFirstTime = 1;
uint8_t blink = 0;


void LCDManagerTask(void* pdata);
void LCDHelloTask(void * parg);
void LCDHistogramTask(void * parg);
void LCD_RxTask(void * parg);

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#define STACK_SIZE_LCD 1024              /*!< Define "taskA" task size */
OS_STK LCD_stk[2][STACK_SIZE_LCD]; /*!< Define "taskA" task stack */

#define STACK_SIZE_LCD_RXTASK 1024
OS_STK LCDRX_stk[2][STACK_SIZE_LCD]; /*!< Define "taskA" task stack */

extern OS_EventID queueAERData; //Recepcion desde el AER

#define LCDDATA_QUEUE_SIZE 512
void *queueLCDDataBuff[LCDDATA_QUEUE_SIZE];
OS_EventID queueLCDData; //Transmision hasta las tareas de pintar

typedef struct {
	uint8_t left_rigth;
	uint8_t channel;
	uint8_t polarity;

}AERData_type;

//Variables de tiempo (en microsegundos)
uint32_t time_init_ref = 0;
uint32_t time_end_ref = 0;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CreateLCD_OSRsrc(void){
	queueLCDData = CoCreateQueue(queueLCDDataBuff, LCDDATA_QUEUE_SIZE, EVENT_SORT_TYPE_FIFO);
}

void decodeAERData(uint8_t AERdataIn, uint8_t* AER_LRChannel, uint8_t* AER_IDChannel, uint8_t* AER_Polarity){
	uint8_t lr = 0;
	uint8_t chann = 0;
	uint8_t polarit = 0;

	lr = AERdataIn & 0x80;
	lr = lr >> 7;

	chann = AERdataIn & 0x7E;
	chann = chann >> 1;

	polarit = AERdataIn & 0x01;

	*AER_LRChannel = lr;
	*AER_IDChannel = chann;
	*AER_Polarity = polarit;
}

void CreateLCDTask(void) {

	LCD_Initialization();
	LCD_Clear(Blue);

	CoCreateTask(LCDManagerTask, 0, 3, &LCD_stk[0][STACK_SIZE_LCD-1], STACK_SIZE_LCD);

}

void LCDManagerTask(void* pdata) {

	OS_TID lcdId;

	for (;;) {

		if (isFirstTime) {
			lcdId = CoCreateTask (LCDHelloTask,0,1,&LCD_stk[1][STACK_SIZE_LCD-1],STACK_SIZE_LCD);
			waitForKey(5, 0);
			CoDelTask(lcdId);
			isFirstTime = 0;
			CoCreateTask(LCD_RxTask, 0, 2,&LCDRX_stk[1][STACK_SIZE_LCD_RXTASK-1],STACK_SIZE_LCD_RXTASK);
		}

		lcdId = CoCreateTask (LCDHistogramTask,0,1,&LCD_stk[1][STACK_SIZE_LCD-1],STACK_SIZE_LCD);
		waitForKey(5, 0);
		CoDelTask(lcdId);

		CoTimeDelay(0, 0, 0, 500);
	}
}

void LCDHelloTask(void * parg) {
	char str[32];
	uint16_t i = 0;
	LCD_Clear(White);
	LCD_PrintText(10, 20, "¡Hola! Estas en el visualizador de ", Blue, White);
	LCD_PrintText(10, 32, "eventos AER ", Blue, White);
	LCD_PrintText(10, 44, "Puedes elegir entre 3 opciones: ", Blue, White);
	LCD_PrintText(10, 56, "Cocleograma: ", Blue, White);
	LCD_PrintText(10, 68, "     -> timeStamp (x)-direccion (y) ", Blue, White);
	LCD_PrintText(10, 80, "Histograma: ", Blue, White);
	LCD_PrintText(10, 92, "     -> direccion (x)-nro.eventos (y) ", Blue, White);
	LCD_PrintText(10, 104, "Cocleograma: ", Blue, White);
	LCD_PrintText(10, 116, "    -> tiempo (x)-canales (y) ", Blue, White);

	for (;;) {

		if (blink) {
			LCD_PrintText(10, 145, "¡Pulse el joystick!", Blue, White);
		} else {
			LCD_PrintText(10, 145, "Pulse el joystick!", White, White);
		}
		blink = !blink;
		CoTimeDelay(0, 0, 0, 500);
	}
}

void LCD_RxTask(void * parg){

	void *msg_in;
	StatusType res_State;

	time_init_ref = 0;
	time_end_ref = time_init_ref + HISTOGRAM_FRAME(10000);

	while(1){

		//Primero leemos un dato de la cola
		msg_in = CoPendQueueMail(queueAERData, 0, &res_State);

		//Metemos estos datos en otra cola que proporcione a las tareas de pintar los datos
		//y que iran cogiendo cuando los necesiten
		CoPostQueueMail(queueLCDData, msg_in);
		CoTimeDelay(0, 0, 0, 1);
		//CoTickDelay(10);
	}
}

void LCDHistogramTask(void * parg) {

	/////Datos leidos de la cola///////
	uint16_t dat = 0;
	uint16_t tim = 0;
	StatusType result;
	void *msg;
	///////////////////////////////////

	//Informacion extraida de la coclea
	AERData_type aerDecodedData;

	//Array para almacenar el numero de eventos por canal
	uint32_t numEvents_channel[NUM_CHANNELS];

	LCD_Clear(White);
	LCD_PrintText(10, 224, "Histogram", Blue, White);
	/* Preparacion de las mallas del eje X e Y */
	LCD_drawXYMesh();
	//Ponemos a 0 todas las posiciones del array para poder hacer el incremento
	AER_clearNumEventsChannelBuffer(numEvents_channel, NUM_CHANNELS);

	// Bucle principal
	for (;;) {

		//Primero cogemos los datos de la cola
		msg = CoPendQueueMail(queueLCDData, 0, &result);
		dat = ((uint16_t*)msg)[0];
		tim = ((uint16_t*)msg)[1];

		//Comprobamos si el paquete está dentro de la ventana temporal
		//if(((uint32_t)tim) >= time_init_ref && ((uint32_t)tim) <= time_end_ref){

			//Decodificamos el dato
			decodeAERData(dat, &aerDecodedData.left_rigth, &aerDecodedData.channel, &aerDecodedData.polarity);

			numEvents_channel[aerDecodedData.channel] = numEvents_channel[aerDecodedData.channel] + 1;

		//}else{
			//Cuando este fuera del rango temporal de la ventana significa que tenemos que pintar
			//Dibujamos las columnas con la altura en funcion del numero de eventos
			//Fijamos el maximo aprox 8500-9000)
			LCD_drawHistogramColumns(numEvents_channel, NUM_CHANNELS);
			//AER_clearNumEventsChannelBuffer(numEvents_channel, NUM_CHANNELS);

		//}

		//
		time_init_ref = tim;
		time_end_ref = time_init_ref + HISTOGRAM_FRAME(10000);

		// cada tick equivale a 50ms
		//CoTimeDelay(0, 0, 0, 1);
		//CoTickDelay(10);
	}

}
