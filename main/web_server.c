/*
	Server WEB Socket.
	This example code is in the Public Domain (or CC0 licensed, at your option.)
	Unless required by applicable law or agreed to in writing, this
	software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
	CONDITIONS OF ANY KIND, either express or implied.
*/

#include <stdio.h>
#include <inttypes.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/message_buffer.h"
#include "esp_log.h"

#include "websocket_server.h"

static QueueHandle_t client_queue;
extern MessageBufferHandle_t xMessageBufferToClient;

const static int client_queue_size = 10;

// handles websocket events
void websocket_callback(uint8_t num,WEBSOCKET_TYPE_t type,char* msg,uint64_t len) 
{
	const static char* TAG = "websocket_callback";
	//int value;

	switch(type) 
	{
		case WEBSOCKET_CONNECT:
			ESP_LOGI(TAG,"Cliente %i conectado.",num);
			break;
		case WEBSOCKET_DISCONNECT_EXTERNAL:
			ESP_LOGI(TAG,"Cliente %i enviou mensagem de desconexao.",num);
			break;
		case WEBSOCKET_DISCONNECT_INTERNAL:
			ESP_LOGI(TAG,"Cliente %i foi desconectado.",num);
			break;
		case WEBSOCKET_DISCONNECT_ERROR:
			ESP_LOGI(TAG,"Cliente %i foi desconectado por um erro.",num);
			break;
		case WEBSOCKET_TEXT:
			if(len) 															// Se o tamanho da mensagem eh maior que zero...
			{ 
				ESP_LOGI(TAG, "got message length %i: %s", (int)len, msg);
				size_t xBytesSent = xMessageBufferSendFromISR(xMessageBufferToClient, msg, len, NULL);
				if (xBytesSent != len) 
				{
					ESP_LOGE(TAG, "xMessageBufferSend fail");
				}
			}
			break;
		case WEBSOCKET_BIN:
			ESP_LOGI(TAG,"Cliente %i sent binary message of size %"PRIu32":\n%s",num,(uint32_t)len,msg);
			break;
		case WEBSOCKET_PING:
			ESP_LOGI(TAG,"Cliente %i pinged us with message of size %"PRIu32":\n%s",num,(uint32_t)len,msg);
			break;
		case WEBSOCKET_PONG:
			ESP_LOGI(TAG,"Cliente %i responded to the ping",num);
			break;
	}
}

static void http_server(struct netconn *conn) 									// Servico para qualquer Cliente.
{
	const static char* TAG = "http_server";
	const static char HTML_HEADER[]  = "HTTP/1.1 200 OK\nContent-type: text/html\n\n";
	const static char ERROR_HEADER[] = "HTTP/1.1 404 Not Found\nContent-type: text/html\n\n";
	const static char JS_HEADER[]    = "HTTP/1.1 200 OK\nContent-type: text/javascript\n\n";
	const static char CSS_HEADER[]   = "HTTP/1.1 200 OK\nContent-type: text/css\n\n";
	// const static char PNG_HEADER[]   = "HTTP/1.1 200 OK\nContent-type: image/png\n\n";
	const static char ICO_HEADER[]   = "HTTP/1.1 200 OK\nContent-type: image/x-icon\n\n";
	const static char TTF_HEADER[]   = "HTTP/1.1 200 OK\nContent-type: font/ttf\n\n";
	//const static char PDF_HEADER[] = "HTTP/1.1 200 OK\nContent-type: application/pdf\n\n";
	//const static char EVENT_HEADER[] = "HTTP/1.1 200 OK\nContent-Type: text/event-stream\nCache-Control: no-cache\nretry: 3000\n\n";
	struct netbuf* inbuf;
	static char* buf;
	static uint16_t buflen;
	static err_t err;

	// index.html
	extern const uint8_t index_html_start[] asm("_binary_index_html_start");
	extern const uint8_t index_html_end[] asm("_binary_index_html_end");
	const uint32_t index_html_len = index_html_end - index_html_start;

	// medidor.js
	extern const uint8_t medidor_js_start[] asm("_binary_medidor_js_start");
	extern const uint8_t medidor_js_end[] asm("_binary_medidor_js_end");
	const uint32_t medidor_js_len = medidor_js_end - medidor_js_start;

	// fonts.css
	extern const uint8_t fonts_css_start[] asm("_binary_fonts_css_start");
	extern const uint8_t fonts_css_end[] asm("_binary_fonts_css_end");
	const uint32_t fonts_css_len = fonts_css_end - fonts_css_start;

	// favicon.ico
	extern const uint8_t favicon_ico_start[] asm("_binary_favicon_ico_start");
	extern const uint8_t favicon_ico_end[] asm("_binary_favicon_ico_end");
	const uint32_t favicon_ico_len = favicon_ico_end - favicon_ico_start;

	// error.html
	extern const uint8_t error_html_start[] asm("_binary_error_html_start");
	extern const uint8_t error_html_end[] asm("_binary_error_html_end");
	const uint32_t error_html_len = error_html_end - error_html_start;

	// gauge.min.js
	extern const uint8_t gauge_min_js_start[] asm("_binary_gauge_min_js_start");
	extern const uint8_t gauge_min_js_end[] asm("_binary_gauge_min_js_end");
	const uint32_t gauge_min_js_len = gauge_min_js_end - gauge_min_js_start;

	// digital.ttf
	extern const uint8_t digital_ttf_start[] asm("_binary_digital_ttf_start");
	extern const uint8_t digital_ttf_end[] asm("_binary_digital_ttf_end");
	const uint32_t digital_ttf_len = digital_ttf_end - digital_ttf_start;

	netconn_set_recvtimeout(conn,1000); // allow a connection timeout of 1 second
	ESP_LOGI(TAG,"reading from Cliente...");
	err = netconn_recv(conn, &inbuf);
	ESP_LOGI(TAG,"read from Cliente");
	if(err==ERR_OK) {
		netbuf_data(inbuf, (void**)&buf, &buflen);
		if(buf) 
		{
			ESP_LOGD(TAG, "buf=[%s]", buf);
			if (strstr(buf,"GET / ") && !strstr(buf,"Upgrade: websocket")) // default page
			{
				ESP_LOGI(TAG,"Sending /");
				netconn_write(conn, HTML_HEADER, sizeof(HTML_HEADER)-1,NETCONN_NOCOPY);
				netconn_write(conn, index_html_start,index_html_len,NETCONN_NOCOPY);
				netconn_close(conn);
				netconn_delete(conn);
				netbuf_delete(inbuf);
			}

			else if(strstr(buf,"GET / ") && strstr(buf,"Upgrade: websocket")) // default page websocket
			{
				ESP_LOGI(TAG,"Requesting websocket on /");
				ws_server_add_client(conn,buf,buflen,"/",websocket_callback);
				netbuf_delete(inbuf);
			}

			else if(strstr(buf,"GET /medidor.js ")) 
			{
				ESP_LOGI(TAG,"Sending /medidor.js");
				netconn_write(conn, JS_HEADER, sizeof(JS_HEADER)-1,NETCONN_NOCOPY);
				netconn_write(conn, medidor_js_start, medidor_js_len,NETCONN_NOCOPY);
				netconn_close(conn);
				netconn_delete(conn);
				netbuf_delete(inbuf);
			}

			else if(strstr(buf,"GET /fonts.css ")) 
			{
				ESP_LOGI(TAG,"Sending /fonts.css");
				netconn_write(conn, CSS_HEADER, sizeof(CSS_HEADER)-1,NETCONN_NOCOPY);
				netconn_write(conn, fonts_css_start, fonts_css_len,NETCONN_NOCOPY);
				netconn_close(conn);
				netconn_delete(conn);
				netbuf_delete(inbuf);
			}

			else if(strstr(buf,"GET /favicon.ico ")) 
			{
				ESP_LOGI(TAG,"Sending favicon.ico");
				netconn_write(conn,ICO_HEADER,sizeof(ICO_HEADER)-1,NETCONN_NOCOPY);
				netconn_write(conn,favicon_ico_start,favicon_ico_len,NETCONN_NOCOPY);
				netconn_close(conn);
				netconn_delete(conn);
				netbuf_delete(inbuf);
			}

			else if(strstr(buf,"GET /gauge.min.js ")) 
			{
				ESP_LOGI(TAG,"Sending gauge.min.js");
				netconn_write(conn, JS_HEADER, sizeof(JS_HEADER)-1,NETCONN_NOCOPY);
				netconn_write(conn, gauge_min_js_start, gauge_min_js_len,NETCONN_NOCOPY);
				netconn_close(conn);
				netconn_delete(conn);
				netbuf_delete(inbuf);
			}

			else if(strstr(buf,"GET /digital.ttf ")) 
			{
				ESP_LOGI(TAG,"Sending digital.ttf");
				netconn_write(conn, TTF_HEADER, sizeof(TTF_HEADER)-1,NETCONN_NOCOPY);
				netconn_write(conn, digital_ttf_start, digital_ttf_len,NETCONN_NOCOPY);
				netconn_close(conn);
				netconn_delete(conn);
				netbuf_delete(inbuf);
			}

			else if(strstr(buf,"GET /")) 
			{
				ESP_LOGE(TAG,"Unknown request, sending error page: %s",buf);
				netconn_write(conn, ERROR_HEADER, sizeof(ERROR_HEADER)-1,NETCONN_NOCOPY);
				netconn_write(conn, error_html_start, error_html_len,NETCONN_NOCOPY);
				netconn_close(conn);
				netconn_delete(conn);
				netbuf_delete(inbuf);
			}

			else 
			{
				ESP_LOGE(TAG,"Unknown request");
				netconn_close(conn);
				netconn_delete(conn);
				netbuf_delete(inbuf);
			}
		}
		else 
		{
			ESP_LOGI(TAG,"Unknown request (empty?...)");
			netconn_close(conn);
			netconn_delete(conn);
			netbuf_delete(inbuf);
		}
	}
	else { // if err==ERR_OK
		ESP_LOGI(TAG,"error on read, closing connection");
		netconn_close(conn);
		netconn_delete(conn);
		netbuf_delete(inbuf);
	}
}

// receives clients from queue, handles them
void server_handle_task(void* pvParameters) {
	const static char* TAG = "server_handle_task";
	struct netconn* conn;
	ESP_LOGI(TAG,"task starting");
	for(;;) {
		xQueueReceive(client_queue,&conn,portMAX_DELAY);
		if(!conn) continue;
		http_server(conn);
	}
	vTaskDelete(NULL);
}

// handles clients when they first connect. passes to a queue
void server_task(void* pvParameters) {
	const static char* TAG = "server_task";
	char *task_parameter = (char *)pvParameters;
	ESP_LOGI(TAG, "Start task_parameter=%s", task_parameter);
	char url[64];
	sprintf(url, "http://%s", task_parameter);
	ESP_LOGI(TAG, "Starting server on %s", url);

	struct netconn *conn, *newconn;
	static err_t err;
	client_queue = xQueueCreate(client_queue_size,sizeof(struct netconn*));
	configASSERT( client_queue );

	
	UBaseType_t PriorityGet = uxTaskPriorityGet(NULL);
	ESP_LOGI(TAG, "PriorityGet=%d", PriorityGet);
	xTaskCreate(&server_handle_task, "server_handle_task", 1024*3, NULL, PriorityGet, NULL);


	conn = netconn_new(NETCONN_TCP);
	netconn_bind(conn,NULL,80);
	netconn_listen(conn);
	ESP_LOGI(TAG,"server listening");
	do {
		err = netconn_accept(conn, &newconn);
		ESP_LOGI(TAG,"new client");
		if(err == ERR_OK) {
			xQueueSendToBack(client_queue,&newconn,portMAX_DELAY);
			//http_server(newconn);
		}
	} while(err == ERR_OK);
	netconn_close(conn);
	netconn_delete(conn);
	ESP_LOGE(TAG,"task ending, rebooting board");
	esp_restart();
}
