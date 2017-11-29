#ifndef _TCP_UTIL_H_
#define _TCP_UTIL_H_

#include <errno.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_log.h"
#include "esp_err.h"
#include "tcp_perf.h"

void tcp_conn(void *pvParameters);

#endif
