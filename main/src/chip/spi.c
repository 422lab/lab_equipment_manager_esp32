/*
 * spi.c
 *
 *  Created on: 2018-02-10 16:38
 *      Author: Jack Chen <redchenjs@live.com>
 */

#include "esp_log.h"

#include "driver/spi_master.h"

#include "board/st7735.h"
#include "board/st7789.h"

#define TAG "spi"

#ifdef CONFIG_ENABLE_GUI
spi_device_handle_t spi1;

void spi1_init(void)
{
    spi_bus_config_t buscfg={
        .miso_io_num = -1,
        .mosi_io_num = CONFIG_SPI_MOSI_PIN,
        .sclk_io_num = CONFIG_SPI_SCLK_PIN,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
#ifdef CONFIG_SCREEN_PANEL_ST7735
        .max_transfer_sz = ST7735_SCREEN_WIDTH*ST7735_SCREEN_HEIGHT*2
#elif defined(CONFIG_SCREEN_PANEL_ST7789)
        .max_transfer_sz = ST7789_SCREEN_WIDTH*ST7789_SCREEN_HEIGHT*2
#endif
    };
    spi_device_interface_config_t devcfg={
        .mode = 0,                                // SPI mode 0
        .spics_io_num = CONFIG_SPI_CS_PIN,        // CS pin
        .clock_speed_hz = 26000000,               // Clock out at 26 MHz
#ifdef CONFIG_SCREEN_PANEL_ST7735
        .queue_size = 6,                          // We want to be able to queue 6 transactions at a time
        .pre_cb = st7735_setpin_dc,               // Specify pre-transfer callback to handle D/C line
#elif defined(CONFIG_SCREEN_PANEL_ST7789)
        .queue_size = 6,                          // We want to be able to queue 6 transactions at a time
        .pre_cb = st7789_setpin_dc,               // Specify pre-transfer callback to handle D/C line
#endif
        .flags = SPI_DEVICE_3WIRE | SPI_DEVICE_HALFDUPLEX
    };
    ESP_ERROR_CHECK(spi_bus_initialize(HSPI_HOST, &buscfg, 1));
    ESP_ERROR_CHECK(spi_bus_add_device(HSPI_HOST, &devcfg, &spi1));
    ESP_LOGI(TAG, "spi-1 initialized.");
}
#endif
