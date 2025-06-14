//This is the main.c for the ESP-32 PCS Replacement system with a simple web ui for viewing the states of pins. 
//For this to properly run must be uploaded with header.h and http_server.c
//if the webserver is unwanted just delete or comment out the call for the setup

#include "header.h"
//AP goes on Meter, STA goes on unit/wellhead

static bool addpeer = false;

int mode;//Change this to set STA/TX/SLAVE=1 or AP/RX/MASTER=0

uint8_t PEER_MAC[6];//Stores MAC address of peer

Data g_data;
returnData g_returndata;

//Callback function for when data is sent and acknowledged
void staSendCB(const uint8_t *mac_addr, esp_now_send_status_t status){
    if(status == ESP_NOW_SEND_SUCCESS){
        printf("Send Success\n");
    }else{
        printf("Send Fail\n");
    }
}


//Callback function for when station recieves data
void staRecvCB(const esp_now_recv_info_t *esp_now_recv_info, const uint8_t *incomingData, int len){
    memcpy(&g_returndata, incomingData, sizeof(g_returndata));
    printf("Meter: %i   Relay: %i\n", g_returndata.meter_state, g_returndata.relay_state);
    staProcessing();
}

//Sends data back to STA when AP recieves
void apReturnData(){
    if(!addpeer){
        esp_now_peer_info_t peerInfo;
        memcpy(peerInfo.peer_addr, PEER_MAC, 6);
        peerInfo.ifidx = WIFI_IF_AP;
        peerInfo.channel = 0;
        peerInfo.encrypt = false;
        esp_now_add_peer(&peerInfo);
        addpeer = true;
    }
    g_returndata.meter_state = gpio_get_level(18);
    g_returndata.relay_state = gpio_get_level(19);
    ESP_ERROR_CHECK_WITHOUT_ABORT(esp_now_send(PEER_MAC, (uint8_t *) &g_returndata, sizeof(g_returndata)));
    apProcessing();
}

//Send callback for AP
void apSendCB(const uint8_t *mac_addr, esp_now_send_status_t status){
    if(status == ESP_NOW_SEND_SUCCESS){
        printf("Send Success\n");
    }else{
        printf("Send Fail\n");
    }
}

//Recieve callback for AP
void apRecvCB(const esp_now_recv_info_t *esp_now_recv_info, const uint8_t *incomingData, int len){
    memcpy(&g_data, incomingData, sizeof(g_data));
    memcpy(&PEER_MAC, esp_now_recv_info->src_addr, 6);
    printf("Plunger: %i   Solenoid: %i\n", g_data.plunger_state, g_data.solenoid_state);
    apReturnData();
}


//Send Data Task, MUST NOT RETURN(infinite loop) MODE = 1
void sendData(void *pvParameter){
    while(1){
        g_data.plunger_state = gpio_get_level(19);
        g_data.solenoid_state = gpio_get_level(18);
        ESP_ERROR_CHECK_WITHOUT_ABORT(esp_now_send(PEER_MAC, (uint8_t *) &g_data, sizeof(g_data)));
        vTaskDelay(500 / portTICK_PERIOD_MS);
    }
}

void staProcessing(){//Process the recieved data for STA
    if(g_returndata.meter_state == 1){
        gpio_set_level(19, 1);
    }else{
        gpio_set_level(19, 0);
    }
}

void apProcessing(){//Process the recieved data for AP
    if(g_data.plunger_state == 0){
        gpio_set_level(19, 1);
    }else{
        gpio_set_level(19, 0);
    }
}

void apPinUDP(void *pvParameter){
    while(1){
        g_returndata.meter_state = gpio_get_level(18);
        g_returndata.relay_state = gpio_get_level(19);
        vTaskDelay(500 / portTICK_PERIOD_MS);
    }
}

void app_main(){
    gpio_config_t modeset = {
            .pin_bit_mask = (1ULL<<21),
            .mode = GPIO_MODE_INPUT,
            .pull_up_en = GPIO_PULLUP_ENABLE,
            .pull_down_en = GPIO_PULLDOWN_DISABLE
        };
    gpio_config(&modeset);
    mode = gpio_get_level(21);
    //Initialize flash memory and WIFI for ESPNOW Communication
    esp_err_t nvserr = nvs_flash_init();
    if(nvserr == ESP_ERR_NVS_NO_FREE_PAGES || nvserr == ESP_ERR_NVS_NEW_VERSION_FOUND){
        ESP_ERROR_CHECK(nvs_flash_erase());
        nvserr = nvs_flash_init();
    }
    ESP_ERROR_CHECK(nvserr);
    esp_netif_init();
    esp_event_loop_create_default();
    
    
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&cfg);
    esp_wifi_set_storage(WIFI_STORAGE_RAM);
    if(mode == 1){
        ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
        esp_netif_create_default_wifi_sta();
        printf("Started as STA\n");
    }else if(mode == 0){
        ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_APSTA));
        esp_netif_create_default_wifi_ap();
        printf("Started as AP\n");
    }
    ESP_ERROR_CHECK(esp_wifi_start());
    if(mode == 0){//Comment out or delete this statement to avoid the setup of the web server
        web_intf();
        }
    ESP_ERROR_CHECK(esp_now_init());
    
    //Scan for nearby AP and grab MAC Address, only runs if TX/STA
    if(mode == 1){
        rescan:
        wifi_scan_config_t scan_config = {.ssid = 0, .bssid = 0, .channel = 0, .show_hidden = true};
        esp_wifi_scan_start(&scan_config, true);
        wifi_ap_record_t wifi_records[4];
        uint16_t max_records = 4; 
        esp_wifi_scan_get_ap_records(&max_records, wifi_records);
        for(int i = 0;i <= max_records;i++){
            char *ssids = (char *)wifi_records[i].ssid;
            char *wantedssid = "ESP";
            char *result = strstr(ssids, wantedssid);
            printf("%32s\n", (char *)wifi_records[i].ssid);
            if(result != NULL){
                memcpy(PEER_MAC, wifi_records[i].bssid, 6);
                printf("Found ESP Device\n");
                printf("%4d  ", wifi_records[i].rssi);
                printf("%02x;%02x;%02x;%02x;%02x;%02x\n", PEER_MAC[0], PEER_MAC[1], PEER_MAC[2], PEER_MAC[3], PEER_MAC[4], PEER_MAC[5]);
                break;
            }else if(result == NULL && i >= max_records){
                i = 0;
                printf("ESP Network Not Found\n");
                goto rescan;
            }
        }
        esp_now_peer_info_t peerInfo;
        memcpy(peerInfo.peer_addr, PEER_MAC, 6);
        peerInfo.ifidx = WIFI_IF_STA;
        peerInfo.channel = 0;
        peerInfo.encrypt = false;
        esp_err_t addpeer = esp_now_add_peer(&peerInfo);
        if(addpeer == ESP_OK){
            printf("Connected to AP\n");
        }else{
            printf("Failed Connecting to AP\n");
        }
        
    }
    //Inputs/Outputs setup
    if(mode == 1){
        gpio_config_t plunger = {
            .pin_bit_mask = (1ULL<<19),
            .mode = GPIO_MODE_INPUT,
            .pull_up_en = GPIO_PULLUP_ENABLE,
            .pull_down_en = GPIO_PULLDOWN_DISABLE 
        };
        gpio_config(&plunger);

        gpio_config_t solenoid = {
            .pin_bit_mask = (1ULL<<18),
            .mode = GPIO_MODE_OUTPUT,
            .pull_up_en = GPIO_PULLUP_DISABLE,
            .pull_down_en = GPIO_PULLDOWN_DISABLE
        };
        gpio_config(&solenoid);
    }
    if(mode == 0){
        gpio_config_t meterin = {
            .pin_bit_mask = (1ULL<<18),
            .mode = GPIO_MODE_INPUT,
            .pull_up_en = GPIO_PULLUP_DISABLE,
            .pull_down_en = GPIO_PULLDOWN_DISABLE
        };
        gpio_config(&meterin);

        gpio_config_t meterout = {
            .pin_bit_mask = (1ULL<<19),
            .mode = GPIO_MODE_OUTPUT,
            .pull_up_en = GPIO_PULLUP_DISABLE,
            .pull_down_en = GPIO_PULLDOWN_DISABLE
        };
        gpio_config(&meterout);
    }


    //Create a periodic Task to send data and register callback functions
    if(mode == 1){
        esp_now_register_send_cb(staSendCB);
        esp_now_register_recv_cb(staRecvCB);
        xTaskCreate(&sendData, "Send_Data", 2048, NULL, 5, NULL);
    }else if(mode == 0){
        esp_now_register_send_cb(apSendCB);
        esp_now_register_recv_cb(apRecvCB);
        xTaskCreate(&apPinUDP, "Pin_Update", 2048, NULL, 5, NULL);
    }
    printf("Startup Done\n");
    
}