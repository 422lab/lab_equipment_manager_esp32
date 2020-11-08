/*
 * http_app_status.h
 *
 *  Created on: 2018-04-06 15:09
 *      Author: Jack Chen <redchenjs@live.com>
 */

#ifndef INC_USER_HTTP_APP_STATUS_H_
#define INC_USER_HTTP_APP_STATUS_H_

#include "user/http_app.h"

#include "esp_http_client.h"

extern esp_err_t http_app_status_event_handler(esp_http_client_event_t *evt);
extern void http_app_status_prepare_data(char *buf, int len);

extern req_code_t http_app_get_code(void);

extern void http_app_update_status(req_code_t code);

#endif /* INC_USER_HTTP_APP_STATUS_H_ */
