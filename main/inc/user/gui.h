/*
 * gui.h
 *
 *  Created on: 2018-02-13 22:57
 *      Author: Jack Chen <redchenjs@live.com>
 */

#ifndef INC_USER_GUI_H_
#define INC_USER_GUI_H_

#include <stdint.h>

typedef enum {
    GUI_MODE_IDX_TIMER   = 0xEE,
    GUI_MODE_IDX_QR_CODE = 0xEF,

    GUI_MODE_IDX_MAX,

    GUI_MODE_IDX_PAUSE = 0xFE,
    GUI_MODE_IDX_OFF   = 0xFF,
} gui_mode_t;

// ani0.gif
extern const char ani0_240x135_gif_ptr[] asm("_binary_ani0_240x135_gif_start");
extern const char ani0_240x135_gif_end[] asm("_binary_ani0_240x135_gif_end");
// ani1.gif
extern const char ani1_240x135_gif_ptr[] asm("_binary_ani1_240x135_gif_start");
extern const char ani1_240x135_gif_end[] asm("_binary_ani1_240x135_gif_end");
// ani2.gif
extern const char ani2_240x135_gif_ptr[] asm("_binary_ani2_240x135_gif_start");
extern const char ani2_240x135_gif_end[] asm("_binary_ani2_240x135_gif_end");
// ani3.gif
extern const char ani3_240x135_gif_ptr[] asm("_binary_ani3_240x135_gif_start");
extern const char ani3_240x135_gif_end[] asm("_binary_ani3_240x135_gif_end");
// ani4.gif
extern const char ani4_240x135_gif_ptr[] asm("_binary_ani4_240x135_gif_start");
extern const char ani4_240x135_gif_end[] asm("_binary_ani4_240x135_gif_end");
// ani5.gif
extern const char ani5_240x135_gif_ptr[] asm("_binary_ani5_240x135_gif_start");
extern const char ani5_240x135_gif_end[] asm("_binary_ani5_240x135_gif_end");
// ani6.gif
extern const char ani6_240x135_gif_ptr[] asm("_binary_ani6_240x135_gif_start");
extern const char ani6_240x135_gif_end[] asm("_binary_ani6_240x135_gif_end");
// ani7.gif
extern const char ani7_240x135_gif_ptr[] asm("_binary_ani7_240x135_gif_start");
extern const char ani7_240x135_gif_end[] asm("_binary_ani7_240x135_gif_end");
// ani8.gif
extern const char ani8_240x135_gif_ptr[] asm("_binary_ani8_240x135_gif_start");
extern const char ani8_240x135_gif_end[] asm("_binary_ani8_240x135_gif_end");

extern void gui_set_mode(uint8_t idx);
extern uint8_t gui_get_mode(void);

extern void gui_init(void);

#endif /* INC_USER_GUI_H_ */
