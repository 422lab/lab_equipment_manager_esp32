/*
 * man.h
 *
 *  Created on: 2020-07-15 18:14
 *      Author: Jack Chen <redchenjs@live.com>
 */

#ifndef INC_USER_MAN_H_
#define INC_USER_MAN_H_

#include <stdint.h>

typedef enum {
    MAN_SYNC_MODE_IDX_OFF = 0x00,
    MAN_SYNC_MODE_IDX_ON  = 0x01
} man_sync_mode_t;

typedef struct {
    char qrcode[33];
    char user_info[11];

    uint8_t current_hour;
    uint8_t current_min;
    uint8_t current_sec;

    uint8_t expire_hour;
    uint8_t expire_min;
    uint8_t expire_sec;

    uint8_t remain_hour;
    uint8_t remain_min;
    uint8_t remain_sec;
} man_info_t;

extern void man_set_qrcode(const char *qrcode);
extern char *man_get_qrcode(void);

extern void man_set_user_info(const char *user_info);
extern void man_set_expire_time(int hour, int min, int sec);

extern man_info_t *man_update_info(void);

extern void man_set_sync_mode(man_sync_mode_t idx);
extern man_sync_mode_t man_get_sync_mode(void);

extern void man_init(void);

#endif /* INC_USER_MAN_H_ */
