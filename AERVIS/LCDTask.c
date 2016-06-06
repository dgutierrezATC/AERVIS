#include <CoOS.h>			              /*!< CoOS header file	         */
#include <stdio.h>
#include "stm32f4xx_conf.h"
#include "LCD.h"
#include "LedTask.h"
#include "BSP.h"

#define LCD_WIDTH 320
#define LCD_HEIGHT 240

//Parametros del Histograma
#define HISTOGRAM_FRAME(x) x*1000 //en ms
#define HISTOGRAM_MESH_WIDTH 320
#define HISTOGRAM_MESH_HEIGHT 205

#define NUM_CHANNELS 64

#define STACK_SIZE_LCD 1024              /*!< Define "taskA" task size */
OS_STK LCD_stk[2][STACK_SIZE_LCD]; /*!< Define "taskA" task stack */

uint8_t LCD_current_rxBuff = 0;
uint8_t LCD_has_rxBuff = 0;

uint8_t LCD_isBusy = 0;

uint8_t isFirstTime = 1;
uint8_t blink = 0;

uint16_t AERdatBuff[2][64];

uint8_t AERdatBuff_index = 0;

/**********************************************************************/
extern OS_EventID semAERRxBuff1;
extern OS_EventID semAERRxBuff2;

extern OS_FlagID flagRxAERBuff_Ready;

extern uint16_t AERdat_rxBuff1[2][64];
extern uint16_t AERdat_rxBuff2[2][64];
/**********************************************************************/

void LCDManagerTask(void* pdata);
void LCDHelloTask(void * parg);
void LCDCochleogramTask(void * parg);
void LCDHistogramTask(void * parg);
void LCDSonogramTask(void * parg);
void Cocleogram(void * parg);

#define STACK_SIZE_LCD 1024              /*!< Define "taskA" task size */
OS_STK LCD_stk[2][STACK_SIZE_LCD]; /*!< Define "taskA" task stack */

void CreateLCDTask(void) {

	Init_AnalogJoy();

	LCD_Initialization();
	LCD_Clear(Blue);

	CoCreateTask(LCDManagerTask, 0, 1, &LCD_stk[0][STACK_SIZE_LCD-1],
			STACK_SIZE_LCD);

}

void LCDManagerTask(void* pdata) {

	OS_TID lcdId;

	for (;;) {

		if (isFirstTime) {
			lcdId =
					CoCreateTask (LCDHelloTask,0,1,&LCD_stk[1][STACK_SIZE_LCD-1],STACK_SIZE_LCD);
			waitForKey(5, 0);
			CoDelTask(lcdId);
			isFirstTime = 0;
		}
		/*
		 lcdId =	CoCreateTask (LCDCochleogramTask,0,1,&LCD_stk[1][STACK_SIZE_LCD-1],STACK_SIZE_LCD);
		 waitForKey(5,0);
		 CoDelTask(lcdId);
		 */
		lcdId =
				CoCreateTask (LCDHistogramTask,0,1,&LCD_stk[1][STACK_SIZE_LCD-1],STACK_SIZE_LCD);
		waitForKey(5, 0);
		CoDelTask(lcdId);

		lcdId =CoCreateTask (LCDSonogramTask,0,1,&LCD_stk[1][STACK_SIZE_LCD-1],STACK_SIZE_LCD);
						waitForKey(5, 0);
						CoDelTask(lcdId);

		lcdId =CoCreateTask (Cocleogram,0,1,&LCD_stk[1][STACK_SIZE_LCD-1],STACK_SIZE_LCD);
		waitForKey(5, 0);
		CoDelTask(lcdId);



	}
}

void LCDHelloTask(void * parg) {
	char str[32];
	uint16_t i = 0;
	LCD_Clear(Red);
	LCD_PrintText(10, 20, "Hola! Estas en el visualizador de ", Blue, White);
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
			LCD_PrintText(10, 145, "Pulse el joystick!", Blue, White);
		} else {
			LCD_PrintText(10, 145, "Pulse el joystick!", Red, Red);
		}
		blink = !blink;
		CoTimeDelay(0, 0, 0, 800);
	}

}

void LCD_selectBuffer(uint16_t rxBuff[2][64]) {
	if (LCD_current_rxBuff == 0) {
		CoPendSem(semAERRxBuff1, 50);
		rxBuff = AERdat_rxBuff1; //-> cambiar!!!!!!!! ¿Cómo pasar un buffer de una tarea a otra?? Cola??
	} else if (LCD_current_rxBuff == 1) {
		CoPendSem(semAERRxBuff2, 50);
		rxBuff = AERdat_rxBuff2;
	}
	LCD_has_rxBuff = 1;
	LCD_current_rxBuff = !LCD_current_rxBuff;
}

void LCD_freeBuffer(void) {
	if (LCD_current_rxBuff == 0) {
		CoPostSem(semAERRxBuff1);
	} else if (LCD_current_rxBuff == 1) {
		CoPostSem(semAERRxBuff2);
	}
	LCD_has_rxBuff = 0;
}

void LCD_drawXYMesh(void) {
	uint16_t i = 0;

	LCD_DrawRectangle(0, 0, 320, 205, 3, Black);
	for (i = 0; i < 10; i++) {
		LCD_DrawLine(i * 32, 0, i * 32, 205, Black);
		LCD_DrawLine(0, i * 20.5, 320, i * 20.5, Black);
	}
}

void LCD_printXYValues(uint16_t x_value, uint16_t y_value) {

	char valorX[32];
	char valorY[32];

	sprintf(valorX, "Valor X: %d  ", x_value);
	// imprimimos por pantalla de Coordenada X
	LCD_PrintText(150, 205, valorX, White, Black);
	// pintamos la cadena "Valor X: " y el valor de la coordenada Y
	sprintf(valorY, "Valor Y: %d  ", y_value);
	// imprimimos por pantalla de Coordenada Y
	LCD_PrintText(150, 225, valorY, White, Black);
}

void LCD_drawHistogramColumns(uint32_t *num_events_channel,
		uint8_t num_channels) {
	uint8_t col_index = 0;
	uint32_t col_alt = 0;
	uint32_t col_width = 0;
	uint32_t x, y;

	col_width = (320 - 4 - (num_channels - 2) * 2) / num_channels;

//	for(col_index = 0; col_index < num_channels; col_index++){
//		col_alt = (205*num_events_channel[col_index]) / 9000;
//		x = 3 + (2 + col_width)*col_index;
//		y = 205 - col_alt - 4;
//		LCD_DrawRectangle(x , y, col_width,col_alt,2,Green);
//		LCD_FillRectangle(x +2, y+2, col_width-3,col_alt-3,Red);
//	}
}

void AER_clearNumEventsChannelBuffer(uint32_t *num_events_channel,
		uint8_t num_channels) {
	uint8_t channel_index = 0;
	;

	for (channel_index = 0; channel_index < num_channels; channel_index++) {
		num_events_channel[channel_index] = 0;
	}
}

//void LCDCochleogramTask(void * parg){
//
//	LCD_Clear(Blue);
//	LCD_PrintText(10,224,"Cochleogram",White,Black);
//
//	//variable event_timeStamp para el valor del timeStamp del evento (eje X)
//	uint16_t event_timeStamp = 0;
//
//    //variable event_address para el valor del address del evento (eje Y)
//	uint16_t event_address = 0;
//
//	uint16_t event_timeStamp_ref = 0;
//
//	uint16_t time = 0;
//
//	/* Preparacion de la pantalla de fondo en negro 320 x 205 */
//	LCD_FillRectangle(0,0,320,205,Black);
//	/* Preparacion de las mallas del eje X e Y */
//	LCD_drawXYMesh();
//
//	// Bucle principal
//	for(;;){
//
//		// cada tick equivale a 50ms
//		CoTimeDelay(0,0,0,1);
//
//		//Primero compruebo si estoy pintando
//		//Si no estoy pintando, espero la bandera que me indique que hay un buffer listo
//		if(!LCD_isBusy){
//			//Primero esperamos a que se levante la bandera indicando que hay un buffer listo
//			CoWaitForSingleFlag(flagRxAERBuff_Ready, 0);
//
//			//Una vez se levante la bandera, tengo que coger el semáforo del buffer correspondiente
//			if(LCD_has_rxBuff){
//				LCD_selectBuffer(AERdatBuff);
//				LCD_isBusy = 1;
//			}
//		}else{
//
//			//Una vez tengamos el buffer, aqui empezamos a leer el buffer, y leemos de la posicion correspondiente
//			event_address = AERdatBuff[0][AERdatBuff_index];
//			event_timeStamp = AERdatBuff[1][AERdatBuff_index];
//
//			//Representamos el punto correspondiente
//			time = event_timeStamp_ref - event_timeStamp;
//			LCD_SetPoint(time, event_address,Green);
//
//			/* Para representar valor de X e Y
//			 * en la parte inferior derecha de la pantalla */
//
//			LCD_printXYValues(event_timeStamp, event_address);
//
//			AERdatBuff_index++;
//		}
//
//
//
//		/* Una vez lleguemos al limite del ancho
//		 * de la pantalla limpiamos la pantalla y
//		 * y inicializamos de nuevo el tiempo */
//		time = ;
//		if(tiempo == 320)
//		{
//			tiempo = 0;
//
//			/* Preparacion de fondo y
//			 * Preparacion de las mallas del eje X e Y */
//			LCD_FillRectangle(0,0,320,205,Black);
//			LCD_drawXYMesh();
//		}
//
//
//
//	}
//}

void LCDHistogramTask(void * parg) {

	LCD_Clear(Blue);
	LCD_PrintText(10, 224, "Histogram", White, Blue);

	//variable event_channel para el canal donde se ha producido el evento (eje X)
	uint16_t event_channel = 0;

	//variable num_events para el valor del numero de eventos del canal (eje Y)
	uint16_t num_events = 0;

	//Variables de tiempo
	uint16_t time_init_ref = 0;
	uint16_t time_end_ref = time_init_ref + HISTOGRAM_FRAME(1000);
	uint16_t time_actual = 0;

	//uint32_t numEvents_channel[NUM_CHANNELS];
	uint32_t numEvents_channel[NUM_CHANNELS] = { 5000, 4000, 1000, 7500, 8000,
			600, 2500, 100, 5000, 4000, 1000, 7500, 8000, 600, 2500, 100, 5000,
			4000, 1000, 7500, 8000, 600, 2500, 100, 5000, 4000, 1000, 7500,
			8000, 600, 2500, 100, 5000, 4000, 1000, 7500, 8000, 600, 2500, 100,
			5000, 4000, 1000, 7500, 8000, 600, 2500, 100, 5000, 4000, 1000,
			7500, 8000, 600, 2500, 100, 5000, 4000, 1000, 7500, 8000, 600, 2500,
			100 };

	/* Preparacion de la pantalla de fondo en negro 320 x 205 */
	LCD_FillRectangle(0, 0, 320, 205, White);
	/* Preparacion de las mallas del eje X e Y */
	LCD_drawXYMesh();

	AERdatBuff_index = 0;

	// Bucle principal
	for (;;) {
		// cada tick equivale a 50ms
		CoTimeDelay(0, 0, 0, 1);

		//Primero compruebo si estoy pintando
		//Si no estoy pintando, espero la bandera que me indique que hay un buffer listo
		if (!LCD_isBusy) {
			//Primero esperamos a que se levante la bandera indicando que hay un buffer listo
			CoWaitForSingleFlag(flagRxAERBuff_Ready, 0);

			//Una vez se levante la bandera, tengo que coger el semáforo del buffer correspondiente
			if (!LCD_has_rxBuff) {
				LCD_selectBuffer(AERdatBuff);
				LCD_isBusy = 1;
			}
		} else {
			//Una vez tengamos el buffer, aqui empezamos a leer el buffer, y leemos de la posicion correspondiente
			if (time_actual <= time_end_ref) {
				event_channel = AERdatBuff[0][AERdatBuff_index];
				numEvents_channel[event_channel] =
						numEvents_channel[event_channel] + 1;
				time_actual = AERdatBuff[1][AERdatBuff_index];
				AERdatBuff_index++;

				if (AERdatBuff_index == 64) {
					if (LCD_has_rxBuff) {
						LCD_freeBuffer();
						LCD_has_rxBuff = 0;
						AERdatBuff_index = 0;
					}
					CoWaitForSingleFlag(flagRxAERBuff_Ready, 0);
					if (!LCD_has_rxBuff) {
						LCD_selectBuffer(AERdatBuff);
					}
				}
			} else {
				//Dibujamos las columnas con la altura en funcion del numero de eventos
				//Fijamos el maximo aprox 8500-9000)
				LCD_drawHistogramColumns(numEvents_channel, NUM_CHANNELS);
				AER_clearNumEventsChannelBuffer(numEvents_channel,
						NUM_CHANNELS);
				time_init_ref = time_actual;
				uint16_t time_end_ref = time_init_ref + HISTOGRAM_FRAME(1000);
			}
		}
	}

}

void LCDSonogramTask(void * parg) {

	//variable event_channel para el canal donde se ha producido el evento (eje X)
	uint16_t event_channel = 0;

	//variable num_events para el valor del numero de eventos del canal (eje Y)
	uint16_t num_events = 0;

	//Variables de tiempo
	uint16_t time_init_ref = 0;
	uint16_t time_end_ref = time_init_ref + 200; //ms
	uint16_t total_time_end_ref = 500;
	uint16_t total_time = 0;
	uint16_t time_actual = 0;
	uint16_t XLcdindex = 0;
	//uint32_t numEvents_channel[NUM_CHANNELS];
	uint32_t numEvents_channel[NUM_CHANNELS] = { 5000, 4000, 1000, 7500, 8000, 600, 2500,
			100, 5000, 4000, 1000, 7500, 8000, 600, 2500, 100, 5000, 4000, 1000,
			7500, 8000, 600, 2500, 100, 5000, 4000, 1000, 7500, 8000, 600, 2500,
			100, 5000, 4000, 1000, 7500, 8000, 600, 2500, 100, 5000, 4000, 1000,
			7500, 8000, 600, 2500, 100, 5000, 4000, 1000, 7500, 8000, 600, 2500,
			100, 5000, 4000, 1000, 7500, 8000, 600, 2500, 100 };

	while (1) {
		ClearScreenSonogram();
		// cada tick equivale a 50ms
		CoTimeDelay(0, 0, 0, 1);

		//Primero compruebo si estoy pintando
		//Si no estoy pintando, espero la bandera que me indique que hay un buffer listo
		if (!LCD_isBusy) {
			//Primero esperamos a que se levante la bandera indicando que hay un buffer listo
			CoWaitForSingleFlag(flagRxAERBuff_Ready, 0);

			//Una vez se levante la bandera, tengo que coger el semáforo del buffer correspondiente
			if (!LCD_has_rxBuff) {
				LCD_selectBuffer(AERdatBuff);
				LCD_isBusy = 1;
			}
		} else {
			//Una vez tengamos el buffer, aqui empezamos a leer el buffer, y leemos de la posicion correspondiente
			if (total_time <= total_time_end_ref) {
				if (time_actual <= time_end_ref) {
					event_channel = AERdatBuff[0][AERdatBuff_index];
					numEvents_channel[event_channel] =
							numEvents_channel[event_channel] + 1;
					time_actual = AERdatBuff[1][AERdatBuff_index];
					AERdatBuff_index++;

					if (AERdatBuff_index == 64) {
						if (LCD_has_rxBuff) {
							LCD_freeBuffer();
							LCD_has_rxBuff = 0;
							AERdatBuff_index = 0;
						}
						CoWaitForSingleFlag(flagRxAERBuff_Ready, 0);
						if (!LCD_has_rxBuff) {
							LCD_selectBuffer(AERdatBuff);
						}

					}
				} else {
					//Dibujamos las columnas con la altura en funcion del numero de eventos
					//Fijamos el maximo aprox 8500-9000)
					SonogramPaint(numEvents_channel, XLcdindex);
					if (XLcdindex == 320) {
						XLcdindex = 0;
					} else {
						XLcdindex++;
					}
					AER_clearNumEventsChannelBuffer(numEvents_channel,
							NUM_CHANNELS);
					time_init_ref = time_actual;
					total_time = time_init_ref + total_time;
					uint16_t time_end_ref = time_init_ref + 200; //200ms
				}
			} else {
				ClearScreenSonogram();
				XLcdindex = 0;
				total_time_end_ref = total_time + 500;
			}
		}
	}

}

void Cocleogram(void * parg){

	LCD_Clear(Blue);
		LCD_PrintText(10, 224, "Cocleogram", White, Blue);
		LCD_DrawLine(10, 220, 10, 0, White);
		LCD_DrawLine(320, 220, 0, 220, White);
		for(;;){

		}
}

void SonogramPaint(uint32_t *num_events_channel, int *paint_index) {
	int i;
	for (i = 0; i < NUM_CHANNELS; i++) {
		if (num_events_channel[i] > 2500)
			LCD_FillCircle(20 + paint_index, 220 - i * 3, 1, Red);
		else if (num_events_channel[i] < 2500)
			LCD_FillCircle(20 + paint_index, 220 - i * 3, 1, Green);
		else
			LCD_FillCircle(20 + paint_index, 220 - i * 3, 1, Blue);
	}

}
void ClearScreenSonogram() {

	LCD_Clear(Blue);
	LCD_PrintText(10, 224, "Sonogram", White, Blue);
	LCD_DrawLine(10, 220, 10, 0, White);
	LCD_DrawLine(320, 220, 0, 220, White);
}
