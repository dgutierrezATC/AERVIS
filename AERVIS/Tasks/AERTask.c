#include "AERTask.h"
#include "LCD.h"
#include "LedTask.h"

/*---------------------------- Symbol Define -------------------------------*/
#define STACK_SIZE_AER 1024              /*!< Define "taskA" task size */
#define AERDATA_QUEUE_SIZE 512

/*---------------------------- Variable Define -------------------------------*/
OS_STK aer_stk[2][STACK_SIZE_AER];	  /*!< Define "taskA" task stack */
void *queueAERDataBuff[AERDATA_QUEUE_SIZE];

OS_EventID queueAERData;

uint16_t AERdat_rxBuff[2]; /*Primer buffer de recepcion del bus AER*/

uint16_t AERdat_timeStamp_ref = 0; /*Debido a que el timestamp es absoluto y tenemos que hacerlo relativo, guardaremos aqui el primer timestamp (para representacion)*/

void AER_RxTask(void * parg);

void CreateAER_OSRsrc(void){
	queueAERData = CoCreateQueue(queueAERDataBuff, AERDATA_QUEUE_SIZE, EVENT_SORT_TYPE_FIFO);
}

void CreateAERTask(void){
	Init_AERbus();
	Init_AERprot();
	Init_AERtimestamp();
	CreateAER_OSRsrc();
	CoCreateTask(AER_RxTask,0,3, &aer_stk[1][STACK_SIZE_AER - 1], STACK_SIZE_AER);
}

void AER_RxTask(void * parg){

	uint8_t reqState = 1;
	uint8_t reqState_new = 1;

	uint16_t aer_data_timestamp = 0;

	uint16_t AERdat_rxData; /*Dato capturado del bus AER*/

	StatusType queueError;

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

			AERdat_rxBuff[0] = AERdat_rxData;
			AERdat_rxBuff[1] = aer_data_timestamp;

			queueError = CoPostQueueMail (queueAERData, (void *)AERdat_rxBuff);

		}else if(reqState_new && !reqState){//Si el reqState_new vale 1 y el reqState vale 0, entonces hay flanco de subida
			AER_upACK();
		}

		reqState = reqState_new;

		//CoTimeDelay(0,0,0,1);
		//CoTickDelay(10);
		//GPIO_WriteBit(GPIOG, GPIO_Pin_14, GPIO_ReadInputDataBit(GPIOG, GPIO_Pin_11));
	}
}
