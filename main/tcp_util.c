#include "tcp_util.h"

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


//this task establish a TCP connection and receive data from TCP
void tcp_conn(void *pvParameters){
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


    while (1) {
	      // total_data = 0;
	     vTaskDelay(1000 / portTICK_RATE_MS);//every 3s
         // send_data("This is a test.\n");

         //send_data("{id,name,progress,100}\n");
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
