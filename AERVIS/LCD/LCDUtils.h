void LCD_drawXYMesh(void);
void LCD_drawHistogramColumns(uint32_t *num_events_channel, uint8_t num_channels, uint16_t col_color);
void AER_clearNumEventsChannelBuffer(uint32_t *num_events_channel, uint8_t num_channels);
void ClearScreenSonogram();
void SonogramPaint(uint32_t num_events_channel[50][64]);
void AER_clearNumEventsChannelMatrix(uint16_t num_events_channel[50][64], uint8_t num_channels, uint16_t num_intervals);
