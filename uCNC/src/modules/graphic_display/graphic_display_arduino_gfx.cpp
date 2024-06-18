#include "../../cnc_hal_config_helper.h"

#ifdef GRAPHIC_DISPLAY_USE_ARDUINO_GFX_LIB
#include <Arduino.h>
#include <Arduino_GFX_Library.h>
#include "Arduino_uCNC_SPI.h"

static Arduino_DataBus *bus;
static Arduino_GFX *gfx;
static void *graphic_port;
static int16_t display_w;
static int16_t display_h;

#ifdef __cplusplus
extern "C"
{
#endif

#include "../../cnc.h"
#include "graphic_display.h"
#include "../softspi.h"
#include "../softi2c.h"
// #include "uCNC_logo.h"
#include "u8g2_font_6x12_tr.h"

#define GRAPHIC_DISPLAY_RST 48

#ifndef GRAPHIC_DISPLAY_SPI_DC
#define GRAPHIC_DISPLAY_SPI_DC DOUT11
#endif
#ifndef GRAPHIC_DISPLAY_BKL
#define GRAPHIC_DISPLAY_BKL DOUT12
#endif
#ifndef GRAPHIC_DISPLAY_RST
#define GRAPHIC_DISPLAY_RST -1
#endif

	int16_t gd_font_height(void)
	{
		return 9;
	}

	void gd_init(display_driver_t *driver, void *port_interface)
	{
		graphic_port = port_interface;
		bus = new Arduino_uCNC_SPI((softspi_port_t *)graphic_port, GRAPHIC_DISPLAY_SPI_DC, GRAPHIC_DISPLAY_SPI_CS, false);
		// bus = new Arduino_ESP32SPI(33, 25, 18, 23, 19, VSPI,false);
		// bus = new Arduino_HWSPI(50, 53);

		driver->init();
		io_set_pinvalue(GRAPHIC_DISPLAY_BKL, 0);
		cnc_delay_ms(50);
		io_set_pinvalue(GRAPHIC_DISPLAY_BKL, 1);
		gfx->begin(20000000);
		cnc_delay_ms(100);
		gfx->fillScreen(BLACK);
		// gfx->setRotation(1);
		
		display_w = driver->width;
		display_h = driver->height;
	}

	void gd_clear()
	{
		// gfx->fillScreen(BLACK);
	}

	void gd_flush()
	{
		// gfx->flush();
	}

	void gd_draw_startup(void)
	{
		// gfx->flush();
		// int16_t x = (display_w - uCNClogo.width) >> 1;
		// int16_t y = (display_h - uCNClogo.height) >> 1;
		// gfx->draw16bitRGBBitmap(x, y, uCNClogo.data, uCNClogo.width, uCNClogo.height);
		// gfx->flush();
		// u8g2_ClearBuffer(U8G2);
		// char buff[SYSTEM_MENU_MAX_STR_LEN];
		// rom_strcpy(buff, __romstr__("µCNC"));
		// u8g2_ClearBuffer(U8G2);
		// u8g2_SetFont(U8G2, u8g2_font_9x15_t_symbols);
		// u8g2_DrawUTF8X2(U8G2, (u8g2_GetDisplayWidth(U8G2) / 2 - u8g2_GetUTF8Width(U8G2, buff)), gd_str_justify_center(), buff);
		// rom_strcpy(buff, __romstr__(("v" CNC_VERSION)));
		// u8g2_SetFont(U8G2, u8g2_font_6x12_tr);
		// u8g2_DrawStr(U8G2, gd_str_align_center(buff), gd_str_justify_end(), buff);
		// u8g2_SendBuffer(U8G2);
		gfx->setFont(u8g2_font_6x12_tr);
		gfx->println("Hello World!");
	}

	void gd_draw_h_line(int16_t y0)
	{
		gfx->writeFastHLine(0, y0, display_w, 0x4bea);
	}

	void gd_draw_rectangle(int16_t x0, int16_t y0, int16_t w, int16_t h)
	{
		gfx->drawRect(x0, y0, w, h, 0x4bea);
	}

	void gd_draw_rectangle_fill(int16_t x0, int16_t y0, int16_t w, int16_t h, bool invert)
	{
		int16_t color = BLACK;
		if (invert)
		{
			color = RED;
		}
		gfx->fillRect(x0, y0, w, h, color);
	}

	void gd_draw_triangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, bool invert)
	{
		int16_t color = BLACK;
		if (invert)
		{
			color = RED;
		}
		gfx->fillTriangle(x0,y0,x1,y1,x2,y2, color);
	}

	void gd_draw_string(int16_t x0, int16_t y0, const char *s)
	{
		gfx->setCursor(x0, y0 + 7);
		gfx->print(s);
	}

	void gd_draw_string_inv(int16_t x0, int16_t y0, const char *s, bool invert)
	{
		// if (invert)
		// {
		// 	u8g2_SetDrawColor(U8G2, 0);
		// }
		// u8g2_DrawStr(U8G2, x0, y0, s);
		// if (invert)
		// {
		// 	u8g2_SetDrawColor(U8G2, 1);
		// }
	}

	void gd_draw_button(int16_t x0, int16_t y0, const char *s, int16_t minw, bool invert, bool frameless)
	{
		// uint8_t mode = (!frameless) ? U8G2_BTN_BW1 : U8G2_BTN_BW0;
		// mode |= (!invert) ? 0 : U8G2_BTN_INV;
		// u8g2_DrawButtonUTF8(U8G2, x0, y0, mode, minw, p_h, p_v, s);
	}

	int16_t gd_str_width(const char *s)
	{
		uint16_t w, h;
		int x, y;
		gfx->getTextBounds(s, 1, 1, &x, &y, &w, &h);
		return w;
	}

	int16_t gd_str_align_start(const char *s)
	{
		return 0;
	}

	int16_t gd_str_align_center(const char *s)
	{
		return ((display_w - gd_str_width(s)) >> 1);
	}

	int16_t gd_str_align_end(const char *s)
	{
		return (display_w - gd_str_width(s));
	}

	int16_t gd_str_justify_start(void)
	{
		return 7;
	}

	int16_t gd_str_justify_center(void)
	{
		return (display_h + gd_font_height()) / 2;
	}

	int16_t gd_str_justify_end(void)
	{
		return display_h - 2;
	}

	static int8_t graphic_last_line_offset;
	int16_t gd_get_line_top(int8_t line)
	{
		// line height plus padding
		int8_t offset = graphic_last_line_offset;
		if (offset < 0)
		{
			offset = display_h - (gd_font_height() + 3) * floor(display_h / (gd_font_height() + 3));
			graphic_last_line_offset = offset;
		}
		return line * (gd_font_height() + 3) + offset;
	}

	/**
	 * Create some U8G2 display drivers
	 */

	void st7796_480x320_spi_init()
	{
		gfx = new Arduino_ST7796(bus, GRAPHIC_DISPLAY_RST, 1, false, 320, 480);
	}

	const display_driver_t gd_st7796_480x320_spi = {
			.width = 480,
			.height = 320,
			.init = &st7796_480x320_spi_init};

	void ili9341_240x320_spi_init()
	{
		gfx = new Arduino_ILI9341(bus, GRAPHIC_DISPLAY_RST, 0, true);
	}

	const display_driver_t gd_ili9341_240x320_spi = {
			.width = 240,
			.height = 320,
			.init = &ili9341_240x320_spi_init};

#ifdef __cplusplus
}
#endif
#endif