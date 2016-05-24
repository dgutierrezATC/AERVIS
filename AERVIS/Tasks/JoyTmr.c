#include <CoOS.h>			              /*!< CoOS header file	         */
#include "BSP.h"


void JoyTimer(void);


OS_FlagID keyFlag[5];


uint8_t waitForKey(uint8_t key, uint32_t timeout){

	CoWaitForSingleFlag(keyFlag[key-1],timeout);

	return key;

}


void  CreateJoyFlags(void){
	uint8_t i;
	for(i=0;i<5;i++){
		keyFlag[i]=CoCreateFlag(1,0);
	}
}


void CreateJoyTask(void){
	OS_TCID joyId;
	Init_Joy();

	joyId=CoCreateTmr(TMR_TYPE_PERIODIC,1,50,JoyTimer);
	CoStartTmr(joyId);


}

void JoyTask (void* pdata){
	uint8_t last_joyKey,key;

	last_joyKey=Read_Joy();
  for (;;) {
	  	key=Read_Joy();
	  	if((key!=0) && (last_joyKey!=key)){
	  		CoSetFlag(keyFlag[key-1]);
	  //		CoClearFlag(keyFlag[key-1]);
	  	}
	  	last_joyKey=key;

		CoTimeDelay (0,0,0,50);
  }
}





uint8_t lastjoyKey;
void JoyTimer(void){
	uint8_t key;
	key=Read_Joy();
  	if((key!=0) && (lastjoyKey!=key)){
		CoSetFlag(keyFlag[key-1]);
 //		CoClearFlag(keyFlag[key-1]);
  	}
  	lastjoyKey=key;
}

