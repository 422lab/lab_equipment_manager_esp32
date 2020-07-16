/*
 * gui.c
 *
 *  Created on: 2018-02-13 22:57
 *      Author: Jack Chen <redchenjs@live.com>
 */

#include <time.h>
#include <string.h>

#include "esp_log.h"
#include "esp_system.h"

#include "gfx.h"
#include "qrcode.h"

#include "core/os.h"
#include "user/man.h"
#include "user/gui.h"
#include "user/http_app_status.h"

#define TAG "gui"

static const char *img_file_ptr[][2] = {
    {ani0_240x135_gif_ptr, ani0_240x135_gif_end}, // "WiFi"
    {ani1_240x135_gif_ptr, ani1_240x135_gif_end}, // "Loading"
    {ani2_240x135_gif_ptr, ani2_240x135_gif_end}, // "Success"
    {ani3_240x135_gif_ptr, ani3_240x135_gif_end}, // "NFC"
    {ani4_240x135_gif_ptr, ani4_240x135_gif_end}, // "PowerOff"
    {ani5_240x135_gif_ptr, ani5_240x135_gif_end}, // "Clock"
    {ani6_240x135_gif_ptr, ani6_240x135_gif_end}, // "Error"
    {ani7_240x135_gif_ptr, ani7_240x135_gif_end}, // "Config"
    {ani8_240x135_gif_ptr, ani8_240x135_gif_end}, // "Updating"
};

GDisplay *gui_gdisp = NULL;

static coord_t gui_disp_width = 0;
static coord_t gui_disp_height = 0;

static GTimer gui_flush_timer;

static uint8_t gui_mode = 0;
static uint8_t gui_backlight = 255;

void printQr(const uint8_t qrcode[])
{
    uint32_t bg_color = White;
    int size = qrcodegen_getSize(qrcode);
    int border = 2;

    EventBits_t uxBits = xEventGroupGetBits(wifi_event_group);
    if (!(uxBits & WIFI_RDY_BIT)) {
        bg_color = Silver;
    } else {
        bg_color = White;
    }

    gdispGClear(gui_gdisp, bg_color);
    gdispGSetBacklight(gui_gdisp, gui_backlight);

    for (int y=-border; y<size+border; y++) {
        for (int x=-border; x<size+border; x++) {
            if (qrcodegen_getModule(qrcode, x, y)) {
                gdispGFillArea(gui_gdisp, x*5+59, y*5+5, 5, 5, Black);
            } else {
                gdispGFillArea(gui_gdisp, x*5+59, y*5+5, 5, 5, bg_color);
            }
        }
    }

    gtimerJab(&gui_flush_timer);
}

static void gui_flush_task(void *pvParameter)
{
    gdispGFlush(gui_gdisp);
}

static void gui_task(void *pvParameter)
{
    font_t gui_font;
    char text_buff[32] = {0};
    portTickType xLastWakeTime;

    gfxInit();

    gui_gdisp = gdispGetDisplay(0);
    gui_font = gdispOpenFont("DejaVuSans32");
    gui_disp_width = gdispGGetWidth(gui_gdisp);
    gui_disp_height = gdispGGetHeight(gui_gdisp);

    gtimerStart(&gui_flush_timer, gui_flush_task, NULL, TRUE, TIME_INFINITE);

    ESP_LOGI(TAG, "started.");

    gdispGSetOrientation(gui_gdisp, GDISP_ROTATE_270);

    while (1) {
        switch (gui_mode) {
        case GUI_MODE_IDX_GIF_WIFI:
        case GUI_MODE_IDX_GIF_BUSY:
        case GUI_MODE_IDX_GIF_DONE:
        case GUI_MODE_IDX_GIF_NFC:
        case GUI_MODE_IDX_GIF_PWR:
        case GUI_MODE_IDX_GIF_CLK:
        case GUI_MODE_IDX_GIF_ERR:
        case GUI_MODE_IDX_GIF_CFG:
        case GUI_MODE_IDX_GIF_UPD: {
            gdispImage gfx_image;

            if (!(gdispImageOpenMemory(&gfx_image, img_file_ptr[gui_mode][0]) & GDISP_IMAGE_ERR_UNRECOVERABLE)) {
                gdispImageSetBgColor(&gfx_image, Black);

                gdispGSetBacklight(gui_gdisp, gui_backlight);

                while (1) {
                    xLastWakeTime = xTaskGetTickCount();

                    if (xEventGroupGetBits(user_event_group) & GUI_RLD_BIT) {
                        xEventGroupClearBits(user_event_group, GUI_RLD_BIT);
                        break;
                    }

                    if (gdispImageDraw(&gfx_image, 0, 0, gfx_image.width, gfx_image.height, 0, 0) != GDISP_IMAGE_ERR_OK) {
                        ESP_LOGE(TAG, "failed to draw image: %u", gui_mode);
                        gui_mode = GUI_MODE_IDX_OFF;
                        break;
                    }

                    gtimerJab(&gui_flush_timer);

                    delaytime_t delay = gdispImageNext(&gfx_image);
                    if (delay == TIME_INFINITE) {
                        gui_mode = GUI_MODE_IDX_PAUSE;
                        break;
                    }

                    if (delay != TIME_IMMEDIATE) {
                        vTaskDelayUntil(&xLastWakeTime, delay / portTICK_RATE_MS);
                    }
                }

                gdispImageClose(&gfx_image);
            } else {
                ESP_LOGE(TAG, "failed to open image: %u", gui_mode);
                gui_mode = GUI_MODE_IDX_OFF;
                break;
            }
            break;
        }
        case GUI_MODE_IDX_TIMER:
            gdispGClear(gui_gdisp, Black);
            gdispGSetBacklight(gui_gdisp, gui_backlight);

            man_info_t *info = man_update_info();

            EventBits_t uxBits = xEventGroupGetBits(wifi_event_group);
            if (!(uxBits & WIFI_RDY_BIT)) {
                snprintf(text_buff, sizeof(text_buff), "(%10s)", info->u_info);
                gdispGFillStringBox(gui_gdisp, 2, 2, 236, 32, text_buff, gui_font, Silver, Black, justifyCenter);
            } else {
                snprintf(text_buff, sizeof(text_buff), "(%10s)", info->u_info);
                gdispGFillStringBox(gui_gdisp, 2, 2, 236, 32, text_buff, gui_font, Yellow, Black, justifyCenter);
            }

            snprintf(text_buff, sizeof(text_buff), "T-N:");
            gdispGFillStringBox(gui_gdisp, 2, 34, 86, 32, text_buff, gui_font, Cyan, Black, justifyLeft);

            snprintf(text_buff, sizeof(text_buff), "%02d:%02d:%02d", info->n_hour, info->n_min, info->n_sec);
            gdispGFillStringBox(gui_gdisp, 88, 34, 150, 32, text_buff, gui_font, Cyan, Black, justifyRight);

            snprintf(text_buff, sizeof(text_buff), "T-E:");
            gdispGFillStringBox(gui_gdisp, 2, 67, 86, 32, text_buff, gui_font, Magenta, Black, justifyLeft);

            snprintf(text_buff, sizeof(text_buff), "%02d:%02d:%02d", info->e_hour, info->e_min, info->e_sec);
            gdispGFillStringBox(gui_gdisp, 88, 67, 150, 32, text_buff, gui_font, Magenta, Black, justifyRight);

            if (info->r_hour <= 0 && info->r_min <= 4) {
                snprintf(text_buff, sizeof(text_buff), "T-R:");
                gdispGFillStringBox(gui_gdisp, 2, 100, 86, 32, text_buff, gui_font, Orange, Black, justifyLeft);

                snprintf(text_buff, sizeof(text_buff), "%02d:%02d:%02d", info->r_hour, info->r_min, info->r_sec);
                gdispGFillStringBox(gui_gdisp, 88, 100, 150, 32, text_buff, gui_font, Orange, Black, justifyRight);
            } else {
                snprintf(text_buff, sizeof(text_buff), "T-R:");
                gdispGFillStringBox(gui_gdisp, 2, 100, 86, 32, text_buff, gui_font, Lime, Black, justifyLeft);

                snprintf(text_buff, sizeof(text_buff), "%02d:%02d:%02d", info->r_hour, info->r_min, info->r_sec);
                gdispGFillStringBox(gui_gdisp, 88, 100, 150, 32, text_buff, gui_font, Lime, Black, justifyRight);
            }

            gtimerJab(&gui_flush_timer);

            xEventGroupSetBits(user_event_group, GUI_DONE_BIT);
            xEventGroupWaitBits(
                user_event_group,
                GUI_RLD_BIT,
                pdTRUE,
                pdFALSE,
                portMAX_DELAY
            );

            break;
        case GUI_MODE_IDX_QR_CODE:
            qrcode_encode(man_get_token());

            xEventGroupWaitBits(
                user_event_group,
                GUI_RLD_BIT,
                pdTRUE,
                pdFALSE,
                portMAX_DELAY
            );

            break;
        case GUI_MODE_IDX_PAUSE:
            xEventGroupWaitBits(
                user_event_group,
                GUI_RLD_BIT,
                pdTRUE,
                pdFALSE,
                portMAX_DELAY
            );

            break;
        case GUI_MODE_IDX_OFF:
        default:
            gdispGSetBacklight(gui_gdisp, 0);

            vTaskDelay(500 / portTICK_RATE_MS);

            gdispGClear(gui_gdisp, Black);
            gtimerJab(&gui_flush_timer);

            xEventGroupWaitBits(
                user_event_group,
                GUI_RLD_BIT,
                pdTRUE,
                pdFALSE,
                portMAX_DELAY
            );

            break;
        }
    }
}

void gui_set_mode(uint8_t idx)
{
    gui_mode = idx;

    xEventGroupSetBits(user_event_group, GUI_RLD_BIT);

    ESP_LOGI(TAG, "mode: 0x%02X", gui_mode);
}

uint8_t gui_get_mode(void)
{
    return gui_mode;
}

void gui_init(void)
{
    xTaskCreatePinnedToCore(gui_task, "guiT", 1920, NULL, 6, NULL, 1);
}
