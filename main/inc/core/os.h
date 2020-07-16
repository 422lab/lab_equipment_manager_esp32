/*
 * os.h
 *
 *  Created on: 2018-03-04 20:07
 *      Author: Jack Chen <redchenjs@live.com>
 */

#ifndef INC_CORE_OS_H_
#define INC_CORE_OS_H_

#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"

typedef enum wifi_event_group_bits {
    WIFI_CFG_BIT = BIT0,
    WIFI_RDY_BIT = BIT1,
} wifi_event_group_bits_t;

typedef enum user_event_group_bits {
    NTP_RUN_BIT = BIT0,
    NTP_RDY_BIT = BIT1,

    MAN_RUN_BIT = BIT2,
    KEY_RUN_BIT = BIT3,

    GUI_RLD_BIT  = BIT4,
    GUI_DONE_BIT = BIT5,

    AUDIO_PLAYER_RUN_BIT  = BIT6,
    AUDIO_PLAYER_IDLE_BIT = BIT7,

    HTTP_APP_OTA_RUN_BIT  = BIT8,
    HTTP_APP_OTA_RDY_BIT  = BIT9,
    HTTP_APP_OTA_FAIL_BIT = BIT10,

    HTTP_APP_STATUS_RUN_BIT  = BIT11,
    HTTP_APP_STATUS_RDY_BIT  = BIT12,
    HTTP_APP_STATUS_FAIL_BIT = BIT13,
} user_event_group_bits_t;

extern EventGroupHandle_t wifi_event_group;
extern EventGroupHandle_t user_event_group;

extern void os_init(void);

#endif /* INC_CORE_OS_H_ */
