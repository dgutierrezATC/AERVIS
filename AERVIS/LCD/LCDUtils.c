#include "All.h"

#define LCD_WIDTH 320
#define LCD_HEIGHT 240
#define HISTOGRAM_MESH_WIDTH 320
#define HISTOGRAM_MESH_HEIGHT 205

void LCD_drawXYMesh(void) {
	uint16_t i = 0;

	LCD_DrawRectangle(0, 0, 320, 205, 3, Black);
	//LCD_FillRectangle(0+3,0+3,320-3, 205-3, White);
	/*for (i = 0; i < 10; i++) {
		LCD_DrawLine(i * 32, 0, i * 32, 205, Black);
		LCD_DrawLine(0, i * 20.5, 320, i * 20.5, Black);
	}*/
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

void LCD_drawHistogramColumns(uint32_t *num_events_channel, uint8_t num_channels, uint16_t col_color) {
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
		LCD_DrawRectangle(x , y, col_width,col_alt,2,col_color);
		//LCD_FillRectangle(x +2, y+2, col_width-3,col_alt-3,Red);
	}
}

void AER_clearNumEventsChannelBuffer(uint32_t *num_events_channel, uint8_t num_channels) {
	uint8_t channel_index = 0;
	;

	for (channel_index = 0; channel_index < num_channels; channel_index++) {
		num_events_channel[channel_index] = 0;
	}
}
void SonogramPaint(uint32_t *num_events_channel, int *paint_index) {

	int i;
	int max_channel_index = 0;
	int max_channel_value = 0;
	int first = 1;
	for (i = 0; i < 63; i++) {
		if (first) {
			max_channel_index = i;
			max_channel_value = num_events_channel[i];
			first = 0;
		} else if (max_channel_value <= num_events_channel[i]) {
			max_channel_index = i;
			max_channel_value = num_events_channel[i];

		}

	}
	for (i = 0; i < 63; i++) {
		if (num_events_channel[i] = max_channel_value)
			LCD_FillCircle(20 + paint_index, 220 - i, 1, Red);
		else if (num_events_channel[i] > max_channel_value/2)
			LCD_FillCircle(20 + paint_index, 220 - i, 1, Yellow);
		else
			LCD_FillCircle(20 + paint_index, 220 - i, 1, Green);
	}

}
void ClearScreenSonogram() {

	LCD_Clear(White);
	LCD_DrawLine(10, 220, 10, 0, Blue);
	LCD_DrawLine(320, 220, 0, 220, Blue);
}
