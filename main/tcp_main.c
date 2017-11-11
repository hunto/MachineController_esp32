/* tcp_perf Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/


/*
tcp_perf example

Using this example to test tcp throughput performance.
esp<->esp or esp<->ap

step1:
    init wifi as AP/STA using config SSID/PASSWORD.

step2:
    create a tcp server/client socket using config PORT/(IP).
    if server: wating for connect.
    if client connect to server.
step3:
    send/receive data to/from each other.
    if the tcp connect established. esp will send or receive data.
    you can see the info in serial output.
*/

#include <errno.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_log.h"
#include "esp_err.h"

#include "tcp_perf.h"
#include "soc/uart_struct.h"
#include "driver/uart.h"
#include "driver/gpio.h"




static const char *TAG2 = "uart_events";

/**
 * This example shows how to use the UART driver to handle special UART events.
 *
 * It also reads data from UART0 directly, and echoes it to console.
 *
 * - port: UART0
 * - rx buffer: on
 * - tx buffer: on
 * - flow control: off
 * - event queue: on
 * - pin assignment: txd(default), rxd(default)
 */

#define EX_UART_NUM UART_NUM_1

#define BUF_SIZE (10000)
static QueueHandle_t uart0_queue;

static void uart_event_task(void *pvParameters)
{
    uart_event_t event;
    size_t buffered_size;
    uint8_t* dtmp = (uint8_t*) malloc(BUF_SIZE);
    for(;;) {
        //Waiting for UART event.
        if(xQueueReceive(uart0_queue, (void * )&event, (portTickType)portMAX_DELAY)) {
            ESP_LOGI(TAG2, "uart[%d] event:", EX_UART_NUM);
            switch(event.type) {
                //Event of UART receving data
                /*We'd better handler data event fast, there would be much more data events than
                other types of events. If we take too much time on data event, the queue might
                be full.
                in this example, we don't process data in event, but read data outside.*/
                case UART_DATA:
                    uart_get_buffered_data_len(EX_UART_NUM, &buffered_size);
                    // TODO: handler data here
                    ESP_LOGI(TAG2, "data, len: %d; buffered len: %d", event.size, buffered_size);
                    break;
                //Event of HW FIFO overflow detected
                case UART_FIFO_OVF:
                    ESP_LOGI(TAG2, "hw fifo overflow\n");
                    //If fifo overflow happened, you should consider adding flow control for your application.
                    //We can read data out out the buffer, or directly flush the rx buffer.
                    uart_flush(EX_UART_NUM);
                    break;
                //Event of UART ring buffer full
                case UART_BUFFER_FULL:
                    ESP_LOGI(TAG2, "ring buffer full\n");
                    //If buffer full happened, you should consider encreasing your buffer size
                    //We can read data out out the buffer, or directly flush the rx buffer.
                    // TODO: encreasing buffer instead of flush
                    uart_flush(EX_UART_NUM);
                    break;
                //Event of UART RX break detected
                case UART_BREAK:
                    ESP_LOGI(TAG2, "uart rx break\n");
                    break;
                //Event of UART parity check error
                case UART_PARITY_ERR:
                    ESP_LOGI(TAG2, "uart parity error\n");
                    break;
                //Event of UART frame error
                case UART_FRAME_ERR:
                    ESP_LOGI(TAG2, "uart frame error\n");
                    break;
                //UART_PATTERN_DET
                case UART_PATTERN_DET:
                    ESP_LOGI(TAG2, "uart pattern detected\n");
                    break;
                //Others
                default:
                    ESP_LOGI(TAG2, "uart event type: %d\n", event.type);
                    break;
            }
        }
    }
    free(dtmp);
    dtmp = NULL;
    vTaskDelete(NULL);
}


void uart_init(void *pvParameters) {
  uart_config_t uart_config = {
     .baud_rate = 115200,
     .data_bits = UART_DATA_8_BITS,
     .parity = UART_PARITY_DISABLE,
     .stop_bits = UART_STOP_BITS_1,
     .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
     .rx_flow_ctrl_thresh = 122,
  };
  //Set UART parameters
  uart_param_config(EX_UART_NUM, &uart_config);
  //Set UART log level
  esp_log_level_set(TAG2, ESP_LOG_INFO);
  //Install UART driver, and get the queue.
  uart_driver_install(EX_UART_NUM, BUF_SIZE * 2, BUF_SIZE * 2, 10, &uart0_queue, 0);

  //Set UART pins (using UART0 default pins ie no changes.)
  uart_set_pin(EX_UART_NUM, GPIO_NUM_17, GPIO_NUM_16, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);

  //Set uart pattern detect function.
  //uart_enable_pattern_det_intr(EX_UART_NUM, '+', 3, 10000, 10, 10);
  //Create a task to handler UART event from ISR
  xTaskCreate(uart_event_task, "uart_event_task", 2048, NULL, 12, NULL);
  //process data
  uint8_t* data = (uint8_t*) malloc(BUF_SIZE);
  do {
      // vTaskDelay(1000 / portTICK_RATE_MS);//every 3s
      // uart_write_bytes(EX_UART_NUM, (const char*)"data\n", 6);
      int len = uart_read_bytes(EX_UART_NUM, data, BUF_SIZE, 100 / portTICK_RATE_MS);
      if(len > 0) {
          ESP_LOGI(TAG, "uart read : %d", len);
	  // TODO:sth handle the commander string
          uart_write_bytes(EX_UART_NUM, (const char*)data, len);
      }
  } while(1);
}



//this task establish a TCP connection and receive data from TCP
static void tcp_conn(void *pvParameters){
    ESP_LOGI(TAG, "task tcp_conn.");
    /*wating for connecting to AP*/
    xEventGroupWaitBits(tcp_event_group, WIFI_CONNECTED_BIT,false, true, portMAX_DELAY);

    ESP_LOGI(TAG, "sta has connected to ap.");

    /*create tcp socket*/
    int socket_ret;

#if EXAMPLE_ESP_TCP_MODE_SERVER
    ESP_LOGI(TAG, "tcp_server will start after 3s...");
    vTaskDelay(3000 / portTICK_RATE_MS);
    ESP_LOGI(TAG, "create_tcp_server.");
    socket_ret = create_tcp_server();
#else /*EXAMPLE_ESP_TCP_MODE_SERVER*/
    ESP_LOGI(TAG, "tcp_client will start after 20s...");
    vTaskDelay(20000 / portTICK_RATE_MS);
    ESP_LOGI(TAG, "create_tcp_client.");
    socket_ret = create_tcp_client();
#endif
    if(socket_ret == ESP_FAIL) {
	      ESP_LOGI(TAG, "create tcp socket error,stop.");
	      vTaskDelete(NULL);
    }

    /*create a task to tx/rx data*/
    // TaskHandle_t tx_rx_task;
// #if EXAMPLE_ESP_TCP_PERF_TX
    // xTaskCreate(&send_data, "send_data", 4096, NULL, 4, &tx_rx_task);
// #else /*EXAMPLE_ESP_TCP_PERF_TX*/
    // xTaskCreate(&recv_data, "recv_data", 4096, NULL, 4, &tx_rx_task);
// #endif
    send_data("This is a test.\n");
    int bps;
    while (1) {
	      // total_data = 0;
	      vTaskDelay(1000 / portTICK_RATE_MS);//every 3s
        send_data("This is a test.\n");
	      // bps = total_data / 3;
      	// if (total_data <= 0) {
      	//     int err_ret = check_working_socket();
      	//     if (err_ret == ECONNRESET || ECONNABORTED) {
      	// 	      ESP_LOGW(TAG, "tcp disconnected... stop.\n");
      	// 	      break;
      	//     }
      	// }
    }
    close_socket();
    // vTaskDelete(tx_rx_task);


    vTaskDelete(NULL);
}



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
    xTaskCreate(&uart_init, "uart_init", 4096, NULL, 5, NULL);h
}
