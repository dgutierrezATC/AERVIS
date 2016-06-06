#include "stm32f4xx_conf.h"

void Init_AERbus(void);
void Init_AERprot(void);
void Init_AERtimestamp(void);
void Init_AER(void);

uint8_t AER_getRequest(void);

void AER_upACK(void);
void AER_downACK(void);

void AER_readBus(uint16_t *aer_data);

//void AER_AddToBuff(uint16_t **aer_dataBuff, uint8_t pos, uint16_t aer_data, uint16_t aer_dataTimeStamp);
void AER_AddToBuff(uint16_t aer_dataBuff[2][64], uint8_t pos, uint16_t aer_data, uint16_t aer_dataTimeStamp);

uint32_t AER_getTimeStamp(void);
