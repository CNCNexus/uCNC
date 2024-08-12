// #include <Arduino.h>
// #include <Arduino_GFX_Library.h>
// #include "lvgl/lvgl.h"
// #include "ui/ui.h"
// #include "Arduino_uCNC_SPI.h"

// #include <stdint.h>
// #include <stdbool.h>
// #include <string.h>
// #include <math.h>

// static Arduino_DataBus *bus;
// static Arduino_GFX *gfx;

// extern "C"
// {
// #include "../../cnc.h"
// #include "../system_menu.h"

// #if (UCNC_MODULE_VERSION < 10990 || UCNC_MODULE_VERSION > 99999)
// #error "This module is not compatible with the current version of µCNC"
// #endif

// #ifndef TFT_DISPLAY_SPI_CS
// #define TFT_DISPLAY_SPI_CS DOUT6
// #endif
// #ifndef TFT_DISPLAY_SPI_DC
// #define TFT_DISPLAY_SPI_DC DOUT32
// #endif
// #ifndef TFT_DISPLAY_BKL
// #define TFT_DISPLAY_BKL DOUT33
// #endif
// #ifndef TFT_DISPLAY_RST
// #define TFT_DISPLAY_RST DOUT34
// #endif

// #ifndef TFT_DISPLAY_SPI_FREQ
// #define TFT_DISPLAY_SPI_FREQ 20000000UL
// #endif

// // buffer
// #define SCREENBUFFER_SIZE_PIXELS 480 * 320 / 10
// 	static lv_color_t buf[SCREENBUFFER_SIZE_PIXELS];

// 	/* Display flushing */
// 	void tft_flush(lv_display_t *disp, const lv_area_t *area, uint8_t *pixelmap)
// 	{
// 		uint32_t w = (area->x2 - area->x1 + 1);
// 		uint32_t h = (area->y2 - area->y1 + 1);

// 		if (LV_COLOR_16_SWAP)
// 		{
// 			size_t len = lv_area_get_size(area);
// 			lv_draw_sw_rgb565_swap(pixelmap, len);
// 		}

// 		gfx->draw16bitRGBBitmap(area->x1, area->y1, (uint16_t *)pixelmap, w, h);

// 		lv_disp_flush_ready(disp);
// 	}

// 	bool tft_display_update(void *args)
// 	{
// 		static bool running = false;

// 		if (!running)
// 		{
// 			running = true;
// 			// uint8_t action = SYSTEM_MENU_ACTION_NONE;
// 			// switch (graphic_display_rotary_encoder_control())
// 			// {
// 			// case GRAPHIC_DISPLAY_SELECT:
// 			// 	action = SYSTEM_MENU_ACTION_SELECT;
// 			// 	// prevent double click
// 			// 	graphic_display_rotary_encoder_pressed = 0;
// 			// 	break;
// 			// case GRAPHIC_DISPLAY_NEXT:
// 			// 	action = SYSTEM_MENU_ACTION_NEXT;
// 			// 	break;
// 			// case GRAPHIC_DISPLAY_PREV:
// 			// 	action = SYSTEM_MENU_ACTION_PREV;
// 			// 	break;
// 			// }

// 			// system_menu_action(action);

// 			// cnc_dotasks();
// 			// // render menu
// 			// system_menu_render();
// 			// cnc_dotasks();
// 			lv_timer_handler();
// 			running = false;
// 		}

// 		return EVENT_CONTINUE;
// 	}
// 	CREATE_EVENT_LISTENER_WITHLOCK(cnc_dotasks, tft_display_update, LISTENER_HWSPI_LOCK);
// #endif
// 	DECL_MODULE(tft_display)
// 	{
// 		bus = new Arduino_uCNC_SPI(&mcu_spi2_port, TFT_DISPLAY_SPI_DC, TFT_DISPLAY_SPI_CS, true);
// 		gfx = new Arduino_ST7796(bus, -1, 1, false);

// 		io_set_pinvalue(TFT_DISPLAY_BKL, 0);
// 		cnc_delay_ms(50);
// 		io_set_pinvalue(TFT_DISPLAY_BKL, 1);
// #if ASSERT_PIN(TFT_DISPLAY_RST)
// 		io_set_output(TFT_DISPLAY_RST);
// 		cnc_delay_ms(200);
// 		io_clear_output(TFT_DISPLAY_RST);
// 		cnc_delay_ms(200);
// 		io_set_output(TFT_DISPLAY_RST);
// #endif
// 		gfx->begin(TFT_DISPLAY_SPI_FREQ);
// 		cnc_delay_ms(100);

// 		lv_init();
// 		static lv_disp_t *disp;
// 		disp = lv_display_create(480, 320);
// 		lv_display_set_buffers(disp, buf, NULL, SCREENBUFFER_SIZE_PIXELS * sizeof(lv_color_t), LV_DISPLAY_RENDER_MODE_PARTIAL);
// 		lv_display_set_flush_cb(disp, tft_flush);

// 		// static lv_indev_t* indev;
// 		// indev = lv_indev_create();
// 		// lv_indev_set_type( indev, LV_INDEV_TYPE_POINTER );
// 		// lv_indev_set_read_cb( indev, my_touchpad_read );

// 		lv_tick_set_cb(mcu_millis);

// 		ui_init();
// 		// STARTS SYSTEM MENU MODULE
// 		system_menu_init();
// #ifdef ENABLE_MAIN_LOOP_MODULES
// 		// ADD_EVENT_LISTENER(cnc_reset, tft_display_start);
// 		// ADD_EVENT_LISTENER(cnc_alarm, tft_display_update);
// 		ADD_EVENT_LISTENER(cnc_dotasks, tft_display_update);
// 		// ADD_EVENT_LISTENER(cnc_io_dotasks, graphic_display_rotary_encoder_control_sample);
// #else
// #warning "Main loop extensions are not enabled. Graphic display card will not work."
// #endif
// 	}
// }