#ifndef _UART_UTIL_H_
#define _UART_UTIL_H_

#include "soc/uart_struct.h"
#include "driver/uart.h"
#include "driver/gpio.h"
#include <errno.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_log.h"
#include "esp_err.h"


enum uart_status {
  UART_FREE, UART_RECEIVE_DATA, UART_RECEIVE_INFO
};


void uart_event_task(void *pvParameters);
void uart_init(void *pvParameters);

void uart_data_handler(uint8_t* data);
void uart_write(const char* data);

#endif
