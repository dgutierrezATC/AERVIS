void LCD_selectBuffer(void);
void CreateLCDTask(void);
void SonogramPaint(uint32_t *num_events_channel);
void ClearScreenSonogram(void);
void decodeAERData(uint8_t AERdataIn, uint8_t* AER_LRChannel, uint8_t* AER_IDChannel, uint8_t* AER_Polarity);
