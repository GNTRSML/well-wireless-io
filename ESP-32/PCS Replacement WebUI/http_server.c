//This is to be compiled with the main.c for the PCS replacement system and header.h
//This script sets up a basic webserver for viewing selected variables and handles all of the server requests
//To access the webpage connect to AP, open a browser and type in device IP,
//if using default settings of ESP-IDF WiFi Driver, IP is most likely 192.168.4.1

#include "header.h"

//The string with HTML code
const char html[] = "<!DOCTYPE html><html><head><meta name=\"viewport\" content=\"width=device.width, initial-scale=1\"><title>Plunger Radio</title><meta http-equiv=\"refresh\" content=\"5\"></head><body><h1 style=\"text-align: center;\">ESP32 Variables</h1><div style=\"text-align: center;\"><strong>This page updates every second, 1 = ON    0 = OFF        Note:When plunger sensor is triggered it will actually go to 0</strong><p id=\"solenoidstate\">Solenoid State: </p><p id=\"plungerstate\">Plunger State: </p><p id=\"meterstate\">Meter State: </p><p id=\"relaystate\">Relay State: </p></div><script>function fetchVariables(){fetch('/variables').then((response)=>response.json()).then((data)=>{var solenoidstate=data.solenoidstate;var plungerstate=data.plungerstate;var meterstate=data.meterstate;var relaystate=data.relaystate;document.getElementById('solenoidstate').innerHTML+=solenoidstate;document.getElementById('plungerstate').innerHTML+=plungerstate;document.getElementById('meterstate').innerHTML+=meterstate;document.getElementById('relaystate').innerHTML+=relaystate});}window.onload=fetchVariables;</script></body></html>";

//Creates a json object with variables for JS in HTML code to read
cJSON *create_json_object(void) {
    cJSON *obj = cJSON_CreateObject();
    cJSON_AddNumberToObject(obj, "solenoidstate", g_data.solenoid_state);
    cJSON_AddNumberToObject(obj, "plungerstate", g_data.plunger_state);
    cJSON_AddNumberToObject(obj, "meterstate", g_returndata.meter_state);
    cJSON_AddNumberToObject(obj, "relaystate", g_returndata.relay_state);
    return obj;
}
static esp_err_t serve_html(httpd_req_t *req){
    httpd_resp_set_type(req, "text/html");
    httpd_resp_send(req, html, HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

static esp_err_t variable_handler(httpd_req_t *req) {
    cJSON *obj = create_json_object();
    char *buf = cJSON_Print(obj);
    
    httpd_resp_set_type(req, "application/json");
    httpd_resp_send(req, buf, HTTPD_RESP_USE_STRLEN);
    cJSON_Delete(obj);
    free(buf);
    return ESP_OK;
  }

httpd_uri_t uri_var = {
    .uri = "/variables",
    .method = HTTP_GET,
    .handler = variable_handler,
    .user_ctx = NULL
};
httpd_uri_t serve_html_uri = {
    .uri = "/",
    .method = HTTP_GET,
    .handler = serve_html,
    .user_ctx = NULL
};

void web_intf() {
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    httpd_handle_t server = NULL;
    if (httpd_start(&server, &config) == ESP_OK)
    {
        httpd_register_uri_handler(server, &uri_var);
        httpd_register_uri_handler(server, &serve_html_uri);
    }
    printf("WebServer Started\n");
}
