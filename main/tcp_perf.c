/* tcp_perf Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include <string.h>
#include <sys/socket.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_wifi.h"
#include "esp_event_loop.h"
#include "esp_log.h"


#include "tcp_perf.h"


/* FreeRTOS event group to signal when we are connected to wifi */
EventGroupHandle_t tcp_event_group;

/*socket*/
static int server_socket = 0;
static struct sockaddr_in server_addr;
static struct sockaddr_in client_addr;
static unsigned int socklen = sizeof(client_addr);
static int connect_socket = 0;

int total_data = 0;

/* for send tx */

int total_pack = 0;
int send_success = 0;
int send_fail = 0;
int delay_classify[5] = { 0 };




static esp_err_t event_handler(void *ctx, system_event_t *event)
{
    switch(event->event_id) {
    case SYSTEM_EVENT_STA_START:
        esp_wifi_connect();
        break;
    case SYSTEM_EVENT_STA_DISCONNECTED:
        esp_wifi_connect();
        xEventGroupClearBits(tcp_event_group, WIFI_CONNECTED_BIT);
        break;
    case SYSTEM_EVENT_STA_CONNECTED:
        break;
    case SYSTEM_EVENT_STA_GOT_IP:
        /* got sta ip*/
    	  ESP_LOGI(TAG, "got ip:%s\n",
		    ip4addr_ntoa(&event->event_info.got_ip.ip_info.ip));
    	  xEventGroupSetBits(tcp_event_group, WIFI_CONNECTED_BIT);
        break;
    case SYSTEM_EVENT_AP_STACONNECTED:
        /* esp32 ap has sta jioned*/
    	  ESP_LOGI(TAG, "station:"MACSTR" join,AID=%d\n",
		        MAC2STR(event->event_info.sta_connected.mac),
		        event->event_info.sta_connected.aid);
    	      xEventGroupSetBits(tcp_event_group, WIFI_CONNECTED_BIT);
    	  break;
    case SYSTEM_EVENT_AP_STADISCONNECTED:
        /* sta disconnected esp32 ap*/
    	  ESP_LOGI(TAG, "station:"MACSTR"leave,AID=%d\n",
		        MAC2STR(event->event_info.sta_disconnected.mac),
		        event->event_info.sta_disconnected.aid);
    	      xEventGroupClearBits(tcp_event_group, WIFI_CONNECTED_BIT);
    	  break;
    default:
        break;
    }
    return ESP_OK;
}

//send data
void send_data(char *databuff)
{
    int len = 0;
    ESP_LOGI(TAG, "start sending...");
    len = send(connect_socket, databuff, strlen(databuff), 0);

	  if(len > 0) {
	  } else {
	      if (LOG_LOCAL_LEVEL >= ESP_LOG_DEBUG) {
	    	     show_socket_error_reason(connect_socket);
	      }
	  } /*if(len > 0)*/
}



/*

void send_data(void *pvParameters)
{
    int len = 0;
    char databuff[EXAMPLE_DEFAULT_PKTSIZE];
    memset(databuff, EXAMPLE_PACK_BYTE_IS, EXAMPLE_DEFAULT_PKTSIZE);
    vTaskDelay(100/portTICK_RATE_MS);
    ESP_LOGI(TAG, "start sending...");

    int sendCount = 10;
    while(1) {
        sendCount--;
        if (sendCount < 0) {
            break;
        }
        vTaskDelay(100/portTICK_RATE_MS);
	      //send function
        // char test[4] = {'t', 'e', 's', 't'};
        strcpy(databuff, "A big test.\n");
    	  // len = send(connect_socket, databuff, EXAMPLE_DEFAULT_PKTSIZE, 0);
        len = send(connect_socket, databuff, strlen(databuff), 0);

	      if(len > 0) {
	          total_data += len;
	      } else {
	          if (LOG_LOCAL_LEVEL >= ESP_LOG_DEBUG) {
	    	        show_socket_error_reason(connect_socket);
	          }
	      }
    }
}

*/







//receive data
void recv_data(void *pvParameters)
{
    int len = 0;
    char databuff[EXAMPLE_DEFAULT_PKTSIZE];
    while (1) {
	len = recv(connect_socket, databuff, EXAMPLE_DEFAULT_PKTSIZE, 0);
	if (len > 0) {
	    total_data += len;
	} else {
            if (LOG_LOCAL_LEVEL >= ESP_LOG_DEBUG) {
	        show_socket_error_reason(connect_socket);
            }
	    vTaskDelay(100 / portTICK_RATE_MS);
	}
    }
}


//use this esp32 as a tcp server. return ESP_OK:success ESP_FAIL:error
esp_err_t create_tcp_server()
{
    ESP_LOGI(TAG, "server socket....port=%d\n", EXAMPLE_DEFAULT_PORT);
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
    	  show_socket_error_reason(server_socket);
	      return ESP_FAIL;
    }
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(EXAMPLE_DEFAULT_PORT);
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
    	show_socket_error_reason(server_socket);
	close(server_socket);
	return ESP_FAIL;
    }
    if (listen(server_socket, 5) < 0) {
    	show_socket_error_reason(server_socket);
	close(server_socket);
	return ESP_FAIL;
    }
    connect_socket = accept(server_socket, (struct sockaddr*)&client_addr, &socklen);
    if (connect_socket<0) {
    	show_socket_error_reason(connect_socket);
	close(server_socket);
	return ESP_FAIL;
    }
    /*connection established，now can send/recv*/
    ESP_LOGI(TAG, "tcp connection established!");
    return ESP_OK;
}
//use this esp32 as a tcp client. return ESP_OK:success ESP_FAIL:error
esp_err_t create_tcp_client()
{
    ESP_LOGI(TAG, "client socket....serverip:port=%s:%d\n",
    		EXAMPLE_DEFAULT_SERVER_IP, EXAMPLE_DEFAULT_PORT);
    connect_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (connect_socket < 0) {
    	show_socket_error_reason(connect_socket);
	return ESP_FAIL;
    }
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(EXAMPLE_DEFAULT_PORT);
    server_addr.sin_addr.s_addr = inet_addr(EXAMPLE_DEFAULT_SERVER_IP);
    ESP_LOGI(TAG, "connecting to server...");
    if (connect(connect_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
    	show_socket_error_reason(connect_socket);
	return ESP_FAIL;
    }
    ESP_LOGI(TAG, "connect to server success!");
    return ESP_OK;
}

//wifi_init_sta
void wifi_init_sta()
{
    tcp_event_group = xEventGroupCreate();

    tcpip_adapter_init();
    ESP_ERROR_CHECK(esp_event_loop_init(event_handler, NULL) );

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    wifi_config_t wifi_config = {
        .sta = {
            .ssid = EXAMPLE_DEFAULT_SSID,
            .password = EXAMPLE_DEFAULT_PWD
        },
    };

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA) );
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config) );
    ESP_ERROR_CHECK(esp_wifi_start() );

    ESP_LOGI(TAG, "wifi_init_sta finished.");
    ESP_LOGI(TAG, "connect to ap SSID:%s password:%s \n",
	    EXAMPLE_DEFAULT_SSID,EXAMPLE_DEFAULT_PWD);
}
//wifi_init_softap
void wifi_init_softap()
{
    tcp_event_group = xEventGroupCreate();

    tcpip_adapter_init();
    ESP_ERROR_CHECK(esp_event_loop_init(event_handler, NULL));

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    wifi_config_t wifi_config = {
        .ap = {
            .ssid = EXAMPLE_DEFAULT_SSID,
            .ssid_len = 0,
            .max_connection=EXAMPLE_MAX_STA_CONN,
            .password = EXAMPLE_DEFAULT_PWD,
            .authmode = WIFI_AUTH_WPA_WPA2_PSK
        },
    };
    if (strlen(EXAMPLE_DEFAULT_PWD) ==0) {
	wifi_config.ap.authmode = WIFI_AUTH_OPEN;
    }

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_AP, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "wifi_init_softap finished.SSID:%s password:%s \n",
    	    EXAMPLE_DEFAULT_SSID, EXAMPLE_DEFAULT_PWD);
}




int get_socket_error_code(int socket)
{
    int result;
    u32_t optlen = sizeof(int);
    if(getsockopt(socket, SOL_SOCKET, SO_ERROR, &result, &optlen) == -1) {
	ESP_LOGE(TAG, "getsockopt failed");
	return -1;
    }
    return result;
}

int show_socket_error_reason(int socket)
{
    int err = get_socket_error_code(socket);
    ESP_LOGW(TAG, "socket error %d %s", err, strerror(err));
    return err;
}

int check_working_socket()
{
    int ret;
#if EXAMPLE_ESP_TCP_MODE_SERVER
    ESP_LOGD(TAG, "check server_socket");
    ret = get_socket_error_code(server_socket);
    if(ret != 0) {
	ESP_LOGW(TAG, "server socket error %d %s", ret, strerror(ret));
    }
    if(ret == ECONNRESET)
	return ret;
#endif
    ESP_LOGD(TAG, "check connect_socket");
    ret = get_socket_error_code(connect_socket);
    if(ret != 0) {
	ESP_LOGW(TAG, "connect socket error %d %s", ret, strerror(ret));
    }
    if(ret != 0)
	return ret;
    return 0;
}

void close_socket()
{
    close(connect_socket);
    close(server_socket);
}
