#include "All.h"

//Parametros del Histograma
#define NUM_CHANNELS 64
#define NUM_INTERVALS 50

uint8_t isFirstTime = 1;
uint8_t blink = 0;

void LCDManagerTask(void* pdata);
void LCDHelloTask(void * parg);
void LCDHistogramTask(void * parg);
void LCDSonogramTask(void * parg);

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#define STACK_SIZE_LCD 2500              /*!< Define "taskA" task size */
OS_STK LCD_stk[2][STACK_SIZE_LCD]; /*!< Define "taskA" task stack */

extern OS_EventID queueAERData; //Recepcion desde el AER

#define LCDDATA_QUEUE_SIZE 512
void *queueLCDDataBuff[LCDDATA_QUEUE_SIZE];
OS_EventID queueLCDData; //Transmision hasta las tareas de pintar

typedef struct {
	uint8_t left_rigth;
	uint8_t channel;
	uint8_t polarity;

} AERData_type;

//Variables de tiempo (en microsegundos)
uint32_t time_init_ref = 0;
uint32_t time_end_ref = 1048560*50;
uint32_t time_absolut = 0;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CreateLCD_OSRsrc(void) {
	queueLCDData = CoCreateQueue(queueLCDDataBuff, LCDDATA_QUEUE_SIZE,
			EVENT_SORT_TYPE_FIFO);
}

void decodeAERData(uint8_t AERdataIn, uint8_t* AER_LRChannel,
		uint8_t* AER_IDChannel, uint8_t* AER_Polarity) {
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

	CoCreateTask(LCDManagerTask, 0, 3, &LCD_stk[0][STACK_SIZE_LCD-1],
			STACK_SIZE_LCD); //Antes -> prioridad 3 (funionando bien histograma)

}

void LCDManagerTask(void* pdata) {

	OS_TID lcdId;

	for (;;) {

		/*if (isFirstTime) {
			lcdId =
					CoCreateTask (LCDHelloTask,0,1,&LCD_stk[1][STACK_SIZE_LCD-1],STACK_SIZE_LCD);
			waitForKey(5, 0);
			CoDelTask(lcdId);
			isFirstTime = 0;
		}*/

//		lcdId = CoCreateTask (LCDHistogramTask,0,1,&LCD_stk[1][STACK_SIZE_LCD-1],STACK_SIZE_LCD);
//		waitForKey(5, 0);
//		CoDelTask(lcdId);

		lcdId =
				CoCreateTask (LCDSonogramTask,0,1,&LCD_stk[1][STACK_SIZE_LCD-1],STACK_SIZE_LCD);
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
	LCD_PrintText(10, 92, "     -> direccion (x)-nro.eventos (y) ", Blue,
			White);
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

void LCDHistogramTask(void * parg) {

	/////Datos leidos de la cola///////
	uint16_t dat = 0;
	uint16_t tim = 0;
	uint16_t tim_old = 0;
	uint16_t tim_offset = 0;
	StatusType result;
	void *msg;
	///////////////////////////////////

	//Informacion extraida de la coclea
	AERData_type aerDecodedData;

	//Array para almacenar el numero de eventos por canal
	uint32_t numEvents_channel[NUM_CHANNELS];

	LCD_Clear(White);
	//LCD_PrintText(10, 224, "Histogram", Blue, White);
	/* Preparacion de las mallas del eje X e Y */
	LCD_drawXYMesh();
	//Ponemos a 0 todas las posiciones del array para poder hacer el incremento
	AER_clearNumEventsChannelBuffer(numEvents_channel, NUM_CHANNELS);

	// Bucle principal
	for (;;) {

		//Primero cogemos los datos de la cola
		msg = CoPendQueueMail(queueAERData, 0, &result);
		dat = ((uint16_t*) msg)[0];
		tim = ((uint16_t*) msg)[1];

		if(tim >= tim_old){
			tim_offset = 0;
		}else{
			tim_offset = 65535-tim_old;
		}
		time_absolut = time_absolut + tim + tim_offset;

		//Comprobamos si el paquete está dentro de la ventana temporal
		if(/*time_absolut >= time_init_ref &&*/ time_absolut <= time_end_ref){
			//Decodificamos el dato
			decodeAERData(dat, &aerDecodedData.left_rigth, &aerDecodedData.channel,&aerDecodedData.polarity);
			numEvents_channel[aerDecodedData.channel] = numEvents_channel[aerDecodedData.channel] + 1;
		}else{
			/* Preparacion de las mallas del eje X e Y */
			LCD_Clear(White);
			LCD_drawXYMesh();

			//Cuando este fuera del rango temporal de la ventana significa que tenemos que pintar
			//Dibujamos las columnas con la altura en funcion del numero de eventos
			//Fijamos el maximo aprox 8500-9000)
			LCD_drawHistogramColumns(numEvents_channel, NUM_CHANNELS, Green);
			AER_clearNumEventsChannelBuffer(numEvents_channel, NUM_CHANNELS);
			time_absolut = 0;
		}
		tim_old = tim;
		// cada tick equivale a 50ms
		//CoTimeDelay(0, 0, 0, 1);
		//CoTickDelay(10);
	}

}

void LCDSonogramTask(void * parg) {
//	/////Datos leidos de la cola///////
	uint16_t dat = 0;
	uint16_t tim = 0;
	uint16_t tim_old = 0;
	uint16_t tim_offset = 0;
//
//
	uint16_t t_init_interval = 1;
	uint16_t t_end_interval = (time_end_ref / NUM_INTERVALS);
	uint16_t interval_index = 0;
	uint16_t interval = (time_end_ref / NUM_INTERVALS);
//
//
	StatusType result;
	void *msg;
//	///////////////////////////////////
//
//	//Informacion extraida de la coclea
	AERData_type aerDecodedData;
//
//	//Array para almacenar el numero de eventos por canal
	uint16_t numEvents_channel[NUM_INTERVALS][NUM_CHANNELS];

//
	ClearScreenSonogram();
//	/* Preparacion de las mallas del eje X e Y */
//
//	//Ponemos a 0 todas las posiciones del array para poder hacer el incremento
	AER_clearNumEventsChannelMatrix(numEvents_channel, NUM_CHANNELS, NUM_INTERVALS);
	//interval_index++;
//
//	// Bucle principal

	while(1) {
//
		//Primero cogemos los datos de la cola
		msg = CoPendQueueMail(queueAERData, 0, &result);
		dat = ((uint16_t*) msg)[0];
		tim = ((uint16_t*) msg)[1];
		if(tim >= tim_old){
			tim_offset = 0;
		}else{
			tim_offset = 65535-tim_old;
		}
		time_absolut = time_absolut + tim + tim_offset;

		//Comprobamos si el paquete está dentro de la ventana temporal
		if (time_absolut <= time_end_ref && interval_index < NUM_INTERVALS) {//periodo de muestreo menor cada columna representa un periodo de integracion
			decodeAERData(dat, &aerDecodedData.left_rigth, &aerDecodedData.channel, &aerDecodedData.polarity);
			if(time_absolut>= t_init_interval && time_absolut <= t_end_interval){
				numEvents_channel[interval_index][aerDecodedData.channel] = numEvents_channel[interval_index][aerDecodedData.channel] + 1;
			}else{
				interval_index++;
				numEvents_channel[interval_index][aerDecodedData.channel] = numEvents_channel[interval_index][aerDecodedData.channel] + 1;

			}
		} else {
			SonogramPaint(numEvents_channel);
			t_init_interval = t_end_interval;
			t_init_interval = (interval_index+1)*interval;


			AER_clearNumEventsChannelMatrix(numEvents_channel, NUM_CHANNELS,NUM_INTERVALS);
			ClearScreenSonogram();
			time_absolut = 0;
			interval_index = 0;
		}
		tim_old = tim;
	}
}





