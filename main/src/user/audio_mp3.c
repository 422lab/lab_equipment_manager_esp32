/*
 * audio_mp3.c
 *
 *  Created on: 2018-02-12 20:13
 *      Author: Jack Chen <redchenjs@live.com>
 */

#include "esp_log.h"
#include "esp_system.h"

#include "freertos/FreeRTOS.h"
#include "driver/i2s.h"

#include "mad.h"
#include "frame.h"
#include "synth.h"
#include "stream.h"

#include "os/core.h"
#include "user/audio_mp3.h"

#define TAG "audio_mp3"

static const char *mp3_file_ptr[][2] = {
    {snd0_mp3_ptr, snd0_mp3_end}, // "叮"
    {snd1_mp3_ptr, snd1_mp3_end}, // "认证成功"
    {snd2_mp3_ptr, snd2_mp3_end}, // "认证失败"
    {snd3_mp3_ptr, snd3_mp3_end}, // "连接失败"
    {snd4_mp3_ptr, snd4_mp3_end}, // "连接超时"
    {snd5_mp3_ptr, snd5_mp3_end}, // "网络故障"
    {snd6_mp3_ptr, snd6_mp3_end}, // "系统故障"
    {snd7_mp3_ptr, snd7_mp3_end}  // "开始配网"
};
static uint8_t mp3_file_index   = 0;
static uint8_t playback_pending = 0;

static void audio_mp3_task_handle(void *pvParameters)
{
    // Allocate structs needed for mp3 decoding
    struct mad_stream *stream = malloc(sizeof(struct mad_stream));
    struct mad_frame  *frame  = malloc(sizeof(struct mad_frame));
    struct mad_synth  *synth  = malloc(sizeof(struct mad_synth));

    if (stream == NULL) { ESP_LOGE(TAG, "malloc(stream) failed"); goto err; }
    if (frame  == NULL) { ESP_LOGE(TAG, "malloc(frame) failed");  goto err; }
    if (synth  == NULL) { ESP_LOGE(TAG, "malloc(synth) failed");  goto err; }

    while (1) {
        xEventGroupWaitBits(
            user_event_group,
            AUDIO_MP3_RUN_BIT,
            pdFALSE,
            pdFALSE,
            portMAX_DELAY
        );

        // Initialize mp3 parts
        mad_stream_init(stream);
        mad_frame_init(frame);
        mad_synth_init(synth);

        mad_stream_buffer(
            stream,
            (const unsigned char *)mp3_file_ptr[mp3_file_index][0],
            mp3_file_ptr[mp3_file_index][1] - mp3_file_ptr[mp3_file_index][0]
        );
        while (1) {
            if (mad_frame_decode(frame, stream) == -1) {
                if (!MAD_RECOVERABLE(stream->error)) {
                    break;
                }
                ESP_LOGE(TAG, "dec err 0x%04x (%s)", stream->error, mad_stream_errorstr(stream));
                continue;
            }
            mad_synth_frame(synth, frame);
        }

        mad_synth_finish(synth);
        mad_frame_finish(frame);
        mad_stream_finish(stream);

        if (playback_pending) {
            playback_pending = 0;
        } else {
            xEventGroupSetBits(user_event_group, AUDIO_MP3_IDLE_BIT);
            xEventGroupClearBits(user_event_group, AUDIO_MP3_RUN_BIT);
        }
    }
err:
    free(synth);
    free(frame);
    free(stream);
    ESP_LOGE(TAG, "task failed");
    esp_restart();
}

void audio_mp3_play(uint8_t filename_index)
{
#ifdef CONFIG_ENABLE_AUDIO_PROMPT
    if (filename_index >= (sizeof(mp3_file_ptr) / 2)) {
        ESP_LOGE(TAG, "invalid filename index");
        return;
    }
    mp3_file_index = filename_index;
    EventBits_t uxBits = xEventGroupGetBits(user_event_group);
    if (uxBits & AUDIO_MP3_RUN_BIT) {
        // Previous playback is still not complete
        playback_pending = 1;
    } else {
        xEventGroupClearBits(user_event_group, AUDIO_MP3_IDLE_BIT);
        xEventGroupSetBits(user_event_group, AUDIO_MP3_RUN_BIT);
    }
#endif
}

void audio_mp3_init(void)
{
    xTaskCreate(audio_mp3_task_handle, "AudioMp3T", 8448, NULL, 8, NULL);
}
