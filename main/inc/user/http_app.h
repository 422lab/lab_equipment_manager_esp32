/*
 * http_app.h
 *
 *  Created on: 2018-02-17 18:51
 *      Author: Jack Chen <redchenjs@live.com>
 */

#ifndef INC_USER_HTTP_APP_H_
#define INC_USER_HTTP_APP_H_

typedef enum {
    HTTP_REQ_CODE_DEV_GET_INFO    = 210, // 设备端获取用户信息
    HTTP_REQ_CODE_DEV_SET_ONLINE  = 211, // 设备端请求允许上机
    HTTP_REQ_CODE_DEV_SET_OFFLINE = 212  // 设备端发送下机通知
} req_code_t;

// cert0.pem
extern const char cert0_pem_ptr[] asm("_binary_cert0_pem_start");
extern const char cert0_pem_end[] asm("_binary_cert0_pem_end");

extern void http_app_init(void);

#endif /* INC_USER_HTTP_APP_H_ */
