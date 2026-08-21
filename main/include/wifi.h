#ifndef WIFI_H
#define WIFI_H
#include <esp_wifi.h>

esp_err_t wifi_init_sta(uint32_t timeout_ms);
#endif
