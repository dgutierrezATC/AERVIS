#include "All.h"

#define LCD_WIDTH 320
#define LCD_HEIGHT 240
#define HISTOGRAM_MESH_WIDTH 320
#define HISTOGRAM_MESH_HEIGHT 205

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

void LCD_drawHistogramColumns(uint32_t *num_events_channel, uint8_t num_channels) {
	uint8_t col_index = 0;
	uint32_t col_alt = 0;
	uint32_t col_width = 0;
	uint32_t x, y;

	col_width = (320 - 4 - (num_channels - 2) * 2) / num_channels;

	for(col_index = 0; col_index < num_channels; col_index++){
		//col_alt = (205*num_events_channel[col_index]) / 9000;
		col_alt = (205*num_events_channel[col_index]/100);
		x = 3 + (2 + col_width)*col_index;
		y = 205 - col_alt - 4;
		LCD_DrawRectangle(x , y, col_width,col_alt,2,Green);
		LCD_FillRectangle(x +2, y+2, col_width-3,col_alt-3,Red);
	}
}

void AER_clearNumEventsChannelBuffer(uint32_t *num_events_channel, uint8_t num_channels) {
	uint8_t channel_index = 0;
	;

	for (channel_index = 0; channel_index < num_channels; channel_index++) {
		num_events_channel[channel_index] = 0;
	}
}
