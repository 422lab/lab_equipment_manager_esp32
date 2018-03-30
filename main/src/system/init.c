/*
 * init.c
 *
 *  Created on: 2018-02-10 16:37
 *      Author: Jack Chen <redchenjs@live.com>
 */

#include "device/nvs.h"
#include "device/bt.h"
#include "device/spi.h"
#include "device/i2s.h"
#include "device/gpio.h"
#include "device/uart.h"
#include "device/wifi.h"
#include "device/blufi.h"
#include "device/spiffs.h"

#include "driver/led.h"
#include "buses/emdev.h"

void device_init(void)
{
    nvs0_init();
    bt0_init();
#if defined(CONFIG_ENABLE_GUI)
    spi1_init();
#endif
#if defined(CONFIG_ENABLE_VOICE_PROMPT)
    i2s0_init();
#endif
    gpio0_init();
    uart1_init();
    wifi0_init();
    blufi0_init();
}

void driver_init(void)
{
    led_init();
    emdev_init();
}
