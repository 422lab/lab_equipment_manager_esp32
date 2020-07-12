// Copyright 2019 Espressif Systems (Shanghai) PTE LTD
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <stdio.h>
#include <esp_err.h>

#include "qrcodegen.h"

#define MAX_QRCODE_VERSION 5

extern void printQr(const uint8_t qrcode[]);

esp_err_t qrcode_encode(const char *text)
{
	enum qrcodegen_Ecc errCorLvl = qrcodegen_Ecc_LOW;
    uint8_t *qrcode, *tempBuffer;
    esp_err_t err = ESP_FAIL;

    qrcode = calloc(1, qrcodegen_BUFFER_LEN_FOR_VERSION(MAX_QRCODE_VERSION));
    if (!qrcode)
        return ESP_ERR_NO_MEM;

    tempBuffer = calloc(1, qrcodegen_BUFFER_LEN_FOR_VERSION(MAX_QRCODE_VERSION));
    if (!tempBuffer) {
        free(qrcode);
        return ESP_ERR_NO_MEM;
    }

	// Make and print the QR Code symbol
	bool ok = qrcodegen_encodeText(text, tempBuffer, qrcode, errCorLvl,
		qrcodegen_VERSION_MIN, MAX_QRCODE_VERSION, qrcodegen_Mask_AUTO, true);
	if (ok) {
		printQr(qrcode);
        err = ESP_OK;
    }

    free(qrcode);
    free(tempBuffer);
    return err;
}
