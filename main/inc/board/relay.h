/*
 * relay.h
 *
 *  Created on: 2020-07-12 18:28
 *      Author: Jack Chen <redchenjs@live.com>
 */

#ifndef INC_BOARD_RELAY_H_
#define INC_BOARD_RELAY_H_

typedef enum {
    RELAY_STATUS_IDX_OFF = 0x00,
    RELAY_STATUS_IDX_ON  = 0x01
} relay_status_t;

extern void relay_set_status(relay_status_t val);
extern relay_status_t relay_get_status(void);

extern void relay_init(void);

#endif /* INC_BOARD_RELAY_H_ */
