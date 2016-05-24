#include <CoOS.h>
#include "AER/AERdriver.h"

void CreateAER_OSRsrc(void);
void CreateAERTask(void);

void AER_selectBuffer(uint16_t rxBuff[2][64]);
void AER_freeBuffer(void);
