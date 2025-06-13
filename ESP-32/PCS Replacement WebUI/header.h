//Header file for ESP-32 PCS Replacment

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <freertos/FreeRTOS.h>
#include <freertos/event_groups.h>
#include <freertos/task.h>
#include <esp_now.h>
#include <esp_wifi.h>
#include <esp_netif.h>
#include <esp_event.h>
#include <esp_log.h>
#include <esp_mac.h>
#include <nvs_flash.h>
#include <esp_http_server.h>
#include <esp_system.h>
#include <lwip/sockets.h>
#include <lwip/sys.h>
#include <lwip/api.h>
#include <lwip/netdb.h>
#include <esp_sntp.h>
#include <driver/gpio.h>
#include <cJSON.h>
#include <time.h>
#include <esp_console.h>


extern int mode;//1 for TX, 0 for RX

typedef struct{//To be updated on every transmission and displayed on webpage
    int min;
    int hour;
    int day;
    int month;
    int year;
}lastTxtime;

extern lastTxtime g_txtime;

typedef struct {
    int plunger_state;
    int solenoid_state;
}Data;

extern Data g_data;

typedef struct {
    int meter_state;
    int relay_state;
}returnData;

extern returnData g_returndata;

void web_intf();
void staProcessing();
void apProcessing();
