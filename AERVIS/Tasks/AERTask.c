#include "AERTask.h"
#include "LCD.h"
#include "LedTask.h"
#include <stdio.h>
#include <string.h>


/*---------------------------- Symbol Define -------------------------------*/
#define STACK_SIZE_AER 512              /*!< Define "taskA" task size */

/*---------------------------- Variable Define -------------------------------*/
OS_STK aer_stk[2][512];	  /*!< Define "taskA" task stack */

OS_EventID semAERRxBuff1;
OS_EventID semAERRxBuff2;

OS_FlagID flagRxAERBuff_Ready;

uint16_t AERdat_rxBuff1[2][64]; /*Primer buffer de recepcion del bus AER*/
uint16_t AERdat_rxBuff2[2][64]; /*Segundo buffer de recepcion del bus AER*/

uint8_t current_rxBuff = 0;
uint8_t has_rxBuff = 0;

uint16_t AERdat_timeStamp_ref = 0; /*Debido a que el timestamp es absoluto y tenemos que hacerlo relativo, guardaremos aqui el primer timestamp (para representacion)*/

void AER_RxTask(void * parg);

void CreateAER_OSRsrc(void){
	semAERRxBuff1 = CoCreateSem(0,1,EVENT_SORT_TYPE_FIFO);
	semAERRxBuff2 = CoCreateSem(0,1,EVENT_SORT_TYPE_FIFO);

	flagRxAERBuff_Ready = CoCreateFlag(0, 0);
}

void CreateAERTask(void){
	//Init_AER();
	Init_AERbus();
	Init_AERprot();
	Init_AERtimestamp();
	CreateAER_OSRsrc();
	CoCreateTask(AER_RxTask,0,3, &aer_stk[1][STACK_SIZE_AER - 1], STACK_SIZE_AER);
}

void AER_selectBuffer(void){
	if(current_rxBuff == 0){
		CoPendSem(semAERRxBuff1, 50);
	}else if(current_rxBuff == 1){
		CoPendSem(semAERRxBuff2, 50);
	}
	has_rxBuff = 1;
}

void AER_addData(uint16_t rxBuff_in[2][64]){
	/*if(current_rxBuff == 0){
		memcpy(AERdat_rxBuff1, rxBuff_in, 64*sizeof(uint16_t)*2);
	}else if(current_rxBuff == 1){
		memcpy(AERdat_rxBuff2, rxBuff_in, 64*sizeof(uint16_t)*2);
	}*/

	/*if(current_rxBuff == 0){
		AERdat_rxBuff1[0][rxBuff_index] = aer_data;
		AERdat_rxBuff1[1][rxBuff_index] = aer_dataTimeStamp;
	}else if(current_rxBuff == 1){
		AERdat_rxBuff2[0][rxBuff_index] = aer_data;
		AERdat_rxBuff2[1][rxBuff_index] = aer_dataTimeStamp;
	}*/

}

void AER_freeBuffer(void){
	if(current_rxBuff == 0){
		CoPostSem(semAERRxBuff1);
	}else if(current_rxBuff == 1){
		CoPostSem(semAERRxBuff2);
	}
	current_rxBuff = !current_rxBuff;
	has_rxBuff = 0;
}

void AER_RxTask(void * parg){

	uint8_t reqState = 1;
	uint8_t reqState_new = 1;

	uint8_t rxBuff_index = 0;
	uint16_t aer_data_timestamp = 0;

	uint16_t AERdat_rxData; /*Dato capturado del bus AER*/

	AER_upACK(); //Aquí o en la inicializacion de los puertos?????????????

	while(1){

		reqState_new = AER_getRequest();

		if(!reqState_new && reqState){ //Si el reqState_new vale 0 y el reqState vale 1, entonces hay flanco de bajada

			//Inmediatamente después de detectar el flanco de bajada, leemos del bus
			AER_readBus(&AERdat_rxData);

			//Y luego guardamos el timeStamp del Timer
			aer_data_timestamp = AER_getTimeStamp(); //Devuelve un uint32. MSB o LSB si vble es 16bits??

			//Una vez leido el dato, es cuando bajamos el ACK
			AER_downACK();

			//Despues tenemos que saber que buffer vamos a usar (sincronizar con los semáforos)
			if(!has_rxBuff){
				AER_selectBuffer();
			}

			//Una vez sepamos el buffer almacenamos los datos en la posicion correcta
			if(current_rxBuff == 0){
				AERdat_rxBuff1[0][rxBuff_index] = AERdat_rxData;
				AERdat_rxBuff1[1][rxBuff_index] = aer_data_timestamp;
			}else if(current_rxBuff == 1){
				AERdat_rxBuff2[0][rxBuff_index] = AERdat_rxData;
				AERdat_rxBuff2[1][rxBuff_index] = aer_data_timestamp;
			}

			//Y actualizamos el indice de escritura del buffer
			rxBuff_index++;

			//Hay que comprobar si se ha llegado al final del buffer, para liberarlo, que lo coja la tarea de pintar, y acceder al otro buffer
			if(rxBuff_index == 64){

				//Liberamos el semáforo del buffer
				if(has_rxBuff){
					AER_freeBuffer();
				}
				//Reiniciamos el indice para empezar a rellenar el siguiente buffer
				rxBuff_index = 0;

				//Subimos la bandera para avisar a la tarea de pintar en la pantalla de que el buffer está listo!
				CoSetFlag(flagRxAERBuff_Ready);
			}

		}else if(reqState_new && !reqState){//Si el reqState_new vale 1 y el reqState vale 0, entonces hay flanco de subida
			AER_upACK();
		}

		reqState = reqState_new;

		CoTimeDelay(0,0,0,1);
	}
}
