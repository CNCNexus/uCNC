// #include <Arduino.h>
// // #include <Arduino_GFX.h>
// #include <Arduino_GFX_Library.h>
// #include "Arduino_uCNC_SPI.h"

// static Arduino_DataBus *bus;
// static Arduino_GFX *gfx;

// extern "C"
// {
// #include <stdint.h>
// #include "../../cnc.h"
// #include "../softspi.h"

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
// #define TFT_DISPLAY_SPI_FREQ 27000000UL
// #endif

// 	HARDSPI(tft_spi, 20000000, 0, mcu_spi2_port);

// 	void tft_init(void)
// 	{
// 		bus = new Arduino_uCNC_SPI(&tft_spi, TFT_DISPLAY_SPI_DC, TFT_DISPLAY_SPI_CS, true);
// 		gfx = new Arduino_ST7796(bus, -1, 1, false);
// 		gfx->begin();
// 		gfx->fillScreen(RGB565_RED);
// 	}

// 	void tft_write(uint16_t x, uint16_t y, uint16_t *data, uint16_t w, uint16_t h)
// 	{
// 		gfx->draw16bitRGBBitmap(x, y, data, w, h);
// 		gfx->flush();
// 	}
// }
