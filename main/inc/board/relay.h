/*
 * relay.h
 *
 *  Created on: 2020-07-12 18:28
 *      Author: Jack Chen <redchenjs@live.com>
 */

#ifndef INC_BOARD_RELAY_H_
#define INC_BOARD_RELAY_H_

extern void relay_set_status(bool val);
extern bool relay_get_status(void);

extern void relay_init(void);

#endif /* INC_BOARD_RELAY_H_ */
