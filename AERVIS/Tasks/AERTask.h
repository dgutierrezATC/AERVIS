#include <CoOS.h>
#include "AER/AERdriver.h"

void CreateAER_OSRsrc(void);
void CreateAERTask(void);

void AER_selectBuffer(void);
void AER_freeBuffer(void);
void AER_addData(uint16_t rxBuff_in[2][64]);
