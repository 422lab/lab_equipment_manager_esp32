/*
 * man.h
 *
 *  Created on: 2020-07-15 18:14
 *      Author: Jack Chen <redchenjs@live.com>
 */

#ifndef INC_USER_MAN_H_
#define INC_USER_MAN_H_

#include <stdint.h>

typedef struct {
    char s_token[33];
    char u_info[11];

    uint8_t n_hour;
    uint8_t n_min;
    uint8_t n_sec;

    uint8_t e_hour;
    uint8_t e_min;
    uint8_t e_sec;

    uint8_t r_hour;
    uint8_t r_min;
    uint8_t r_sec;
} man_info_t;

extern void man_set_token(const char *s_token);
extern void man_set_user_info(const char *u_info);
extern void man_set_exp_time(int hour, int min, int sec);

extern char *man_get_token(void);
extern man_info_t *man_update_info(void);

extern void man_init(void);

#endif /* INC_USER_MAN_H_ */
