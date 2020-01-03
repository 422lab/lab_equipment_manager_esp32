/*
 * gui.h
 *
 *  Created on: 2018-02-13 22:57
 *      Author: Jack Chen <redchenjs@live.com>
 */

#ifndef INC_USER_GUI_H_
#define INC_USER_GUI_H_

#include <stdint.h>

#ifdef CONFIG_SCREEN_PANEL_ST7735
// ani0.gif
extern const char ani0_160x80_gif_ptr[] asm("_binary_ani0_160x80_gif_start");
extern const char ani0_160x80_gif_end[] asm("_binary_ani0_160x80_gif_end");
// ani1.gif
extern const char ani1_160x80_gif_ptr[] asm("_binary_ani1_160x80_gif_start");
extern const char ani1_160x80_gif_end[] asm("_binary_ani1_160x80_gif_end");
// ani2.gif
extern const char ani2_160x80_gif_ptr[] asm("_binary_ani2_160x80_gif_start");
extern const char ani2_160x80_gif_end[] asm("_binary_ani2_160x80_gif_end");
// ani3.gif
extern const char ani3_160x80_gif_ptr[] asm("_binary_ani3_160x80_gif_start");
extern const char ani3_160x80_gif_end[] asm("_binary_ani3_160x80_gif_end");
// ani4.gif
extern const char ani4_160x80_gif_ptr[] asm("_binary_ani4_160x80_gif_start");
extern const char ani4_160x80_gif_end[] asm("_binary_ani4_160x80_gif_end");
// ani5.gif
extern const char ani5_160x80_gif_ptr[] asm("_binary_ani5_160x80_gif_start");
extern const char ani5_160x80_gif_end[] asm("_binary_ani5_160x80_gif_end");
// ani6.gif
extern const char ani6_160x80_gif_ptr[] asm("_binary_ani6_160x80_gif_start");
extern const char ani6_160x80_gif_end[] asm("_binary_ani6_160x80_gif_end");
// ani7.gif
extern const char ani7_160x80_gif_ptr[] asm("_binary_ani7_160x80_gif_start");
extern const char ani7_160x80_gif_end[] asm("_binary_ani7_160x80_gif_end");
// ani8.gif
extern const char ani8_160x80_gif_ptr[] asm("_binary_ani8_160x80_gif_start");
extern const char ani8_160x80_gif_end[] asm("_binary_ani8_160x80_gif_end");
#elif defined(CONFIG_SCREEN_PANEL_ST7789)
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
#endif

extern void gui_show_image(uint8_t idx);

extern void gui_init(void);

#endif /* INC_USER_GUI_H_ */
