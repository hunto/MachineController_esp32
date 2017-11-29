#include <errno.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_log.h"
#include "esp_err.h"
#include "tcp_util.h"
#include "uart_util.h"
#include "esp_types.h"
#include "data.h"

DataStructure head = NULL;

void app_main(void)
{
#if EXAMPLE_ESP_WIFI_MODE_AP
    ESP_LOGI(TAG, "EXAMPLE_ESP_WIFI_MODE_AP");
    wifi_init_softap();
#else
    ESP_LOGI(TAG, "ESP_WIFI_MODE_STA");
    wifi_init_sta();
#endif /*EXAMPLE_ESP_WIFI_MODE_AP*/
    xTaskCreate(&tcp_conn, "tcp_conn", 4096, NULL, 5, NULL);
    xTaskCreate(&uart_init, "uart_init", 4096, NULL, 5, NULL);
}
