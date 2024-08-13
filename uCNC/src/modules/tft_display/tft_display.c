/*
	Name: tft_display.c
	Description: Graphic LCD module for µCNC using u8g2 lib.

	Copyright: Copyright (c) João Martins
	Author: João Martins
	Date: 08-09-2022

	µCNC is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version. Please see <http://www.gnu.org/licenses/>

	µCNC is distributed WITHOUT ANY WARRANTY;
	Also without the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
	See the	GNU General Public License for more details.
*/

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>
#include "../../cnc.h"
#include "../system_menu.h"
#include "../softspi.h"
#include "lvgl/lvgl.h"
// #include "lvgl/src/drivers/display/st7796/lv_st7796.h"
#include "ui/ui.h"

#if (UCNC_MODULE_VERSION < 10990 || UCNC_MODULE_VERSION > 99999)
#error "This module is not compatible with the current version of µCNC"
#endif

#ifndef TFT_DISPLAY_SPI_CS
#define TFT_DISPLAY_SPI_CS DOUT6
#endif
#ifndef TFT_DISPLAY_SPI_DC
#define TFT_DISPLAY_SPI_DC DOUT32
#endif
#ifndef TFT_DISPLAY_BKL
#define TFT_DISPLAY_BKL DOUT33
#endif
#ifndef TFT_DISPLAY_RST
#define TFT_DISPLAY_RST DOUT34
#endif

#ifndef TFT_DISPLAY_SPI_FREQ
#define TFT_DISPLAY_SPI_FREQ 20000000UL
#endif

#ifndef TFT_H_RES
#define TFT_H_RES 480
#endif
#ifndef TFT_V_RES
#define TFT_V_RES 320
#endif

// HARDSPI(tft_spi, 20000000, 0, mcu_spi2_port);
extern void tft_init(void);
extern void tft_write(uint16_t x, uint16_t y, uint16_t *data, uint16_t w, uint16_t h);
static lv_disp_t *disp;

// buffer
#define SCREENBUFFER_SIZE_PIXELS (TFT_H_RES * TFT_V_RES / 10)
static lv_color_t buf[SCREENBUFFER_SIZE_PIXELS];

#ifdef ENABLE_MAIN_LOOP_MODULES
bool tft_display_update(void *args)
{
	static bool running = false;

	if (!running)
	{
		running = true;
		// uint8_t action = SYSTEM_MENU_ACTION_NONE;
		// switch (graphic_display_rotary_encoder_control())
		// {
		// case GRAPHIC_DISPLAY_SELECT:
		// 	action = SYSTEM_MENU_ACTION_SELECT;
		// 	// prevent double click
		// 	graphic_display_rotary_encoder_pressed = 0;
		// 	break;
		// case GRAPHIC_DISPLAY_NEXT:
		// 	action = SYSTEM_MENU_ACTION_NEXT;
		// 	break;
		// case GRAPHIC_DISPLAY_PREV:
		// 	action = SYSTEM_MENU_ACTION_PREV;
		// 	break;
		// }

		// system_menu_action(action);

		// cnc_dotasks();
		// // render menu
		// system_menu_render();
		// cnc_dotasks();
		lv_timer_handler();
		running = false;
	}

	return EVENT_CONTINUE;
}
CREATE_EVENT_LISTENER_WITHLOCK(cnc_dotasks, tft_display_update, LISTENER_HWSPI_LOCK);
#endif

// based on https://docs.lvgl.io/master/integration/driver/display/lcd_stm32_guide.html#lcd-stm32-guide
/* Platform-specific implementation of the LCD send command function. In general this should use polling transfer. */
// static void tft_send_cmd(lv_display_t *disp, const uint8_t *cmd, size_t cmd_size, const uint8_t *param, size_t param_size)
// {
// 	LV_UNUSED(disp);
// 	tft_spi.spiconfig.enable_dma = 0;
// 	softspi_start(&tft_spi);
// 	/* DCX low (command) */
// 	io_clear_output(TFT_DISPLAY_SPI_DC);
// 	/* CS low */
// 	io_clear_output(TFT_DISPLAY_SPI_CS);
// 	/* send command */
// 	softspi_bulk_xmit(&tft_spi, cmd, NULL, (uint16_t)cmd_size);

// 	/* DCX high (data) */
// 	io_set_output(TFT_DISPLAY_SPI_DC);
// 	/* for short data blocks we use polling transfer */
// 	softspi_bulk_xmit(&tft_spi, param, NULL, (uint16_t)param_size);
// 	/* CS high */
// 	io_set_output(TFT_DISPLAY_SPI_CS);
// 	softspi_stop(&tft_spi);
// }

/* Platform-specific implementation of the LCD send color function. For better performance this should use DMA transfer.
 * In case of a DMA transfer a callback must be installed to notify LVGL about the end of the transfer.
 */
// static void tft_send_color(lv_display_t *disp, const uint8_t *cmd, size_t cmd_size, uint8_t *param, size_t param_size)
// {
// 	LV_UNUSED(disp);

// 	softspi_start(&tft_spi);
// 	/* DCX low (command) */
// 	io_clear_output(TFT_DISPLAY_SPI_DC);
// 	/* CS low */
// 	io_clear_output(TFT_DISPLAY_SPI_CS);
// 	/* send command */
// 	softspi_bulk_xmit(&tft_spi, cmd, NULL, (uint16_t)cmd_size);
// 	softspi_stop(&tft_spi);
// 	tft_spi.spiconfig.enable_dma = 1;
// 	/* DCX high (data) */
// 	io_set_output(TFT_DISPLAY_SPI_DC);
// 	softspi_start(&tft_spi);
// 	/* for short data blocks we use polling transfer */
// 	softspi_bulk_xmit(&tft_spi, param, NULL, (uint16_t)param_size);
// 	/* CS high */
// 	io_set_output(TFT_DISPLAY_SPI_CS);
// 	softspi_stop(&tft_spi);
// }

void tft_flush_cb(lv_display_t *display, const lv_area_t *area, uint8_t *px_map)
{
	tft_write(area->x1, area->y1, (uint16_t *)px_map, area->x2 - area->x1, area->y2 - area->y1);
	/* IMPORTANT!!!
	 * Inform LVGL that you are ready with the flushing and buf is not used anymore*/
	lv_display_flush_ready(display);
}

DECL_MODULE(tft_display)
{
	io_clear_output(TFT_DISPLAY_BKL);
	cnc_delay_ms(50);
	io_set_output(TFT_DISPLAY_BKL);
#if ASSERT_PIN(TFT_DISPLAY_RST)
	io_set_output(TFT_DISPLAY_RST);
	cnc_delay_ms(100);
	io_clear_output(TFT_DISPLAY_RST);
	cnc_delay_ms(100);
	io_set_output(TFT_DISPLAY_RST);
#endif
	io_set_output(TFT_DISPLAY_SPI_DC);
	io_set_output(TFT_DISPLAY_SPI_CS);
	cnc_delay_ms(100);
	tft_init();

	lv_init();

	disp = lv_display_create(TFT_H_RES, TFT_V_RES);
	// disp = lv_st7796_create(TFT_H_RES, TFT_V_RES, 0, tft_send_cmd, tft_send_color);
	// lv_display_set_rotation(disp, LV_DISPLAY_ROTATION_270);
	lv_display_set_flush_cb(disp, tft_flush_cb);

	lv_display_set_buffers(disp, buf, NULL, SCREENBUFFER_SIZE_PIXELS * sizeof(lv_color_t), LV_DISPLAY_RENDER_MODE_PARTIAL);

	// static lv_indev_t* indev;
	// indev = lv_indev_create();
	// lv_indev_set_type( indev, LV_INDEV_TYPE_POINTER );
	// lv_indev_set_read_cb( indev, my_touchpad_read );
	

	lv_tick_set_cb(mcu_millis);

	ui_init();
	// STARTS SYSTEM MENU MODULE
	system_menu_init();
#ifdef ENABLE_MAIN_LOOP_MODULES
	// ADD_EVENT_LISTENER(cnc_reset, tft_display_start);
	// ADD_EVENT_LISTENER(cnc_alarm, tft_display_update);
	ADD_EVENT_LISTENER(cnc_dotasks, tft_display_update);
	// ADD_EVENT_LISTENER(cnc_io_dotasks, graphic_display_rotary_encoder_control_sample);
#else
#warning "Main loop extensions are not enabled. Graphic display card will not work."
#endif
}
