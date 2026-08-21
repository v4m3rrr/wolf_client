#include "wifi.h"

#include "esp_netif_ip_addr.h"
#include "esp_netif_types.h"
#include <esp_log.h>
#include <esp_wifi.h>

static EventGroupHandle_t s_wifi_event_group;

#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT BIT1

static const char *TAG = "wifi station";
static int s_retry_num = 0;

static void event_handler(void *arg, esp_event_base_t event_base,
			  int32_t event_id, void *event_data)
{
	if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
		esp_wifi_connect();
	} else if (event_base == WIFI_EVENT &&
		   event_id == WIFI_EVENT_STA_DISCONNECTED) {
		if (s_retry_num < CONFIG_APP_WIFI_MAXIMUM_RETRY) {
			esp_wifi_connect();
			s_retry_num++;
			ESP_LOGI(TAG, "retry to connect to the AP");
		} else {
			xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
		}
		ESP_LOGI(TAG, "connect to the AP fail");
	} else if (event_base == IP_EVENT &&
		   event_id == ESP_NETIF_IP_EVENT_LOST_IP) {
		ESP_LOGW(TAG, "Lost IP address, connection degraded");
		xEventGroupClearBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
		if (s_retry_num < CONFIG_APP_WIFI_MAXIMUM_RETRY) {
			esp_wifi_disconnect();
			s_retry_num++;
			esp_wifi_connect();
			ESP_LOGI(TAG, "retrying connection after lost IP");
		} else
			xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);

	} else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
		ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
		ESP_LOGI(TAG, "got ip:" IPSTR, IP2STR(&event->ip_info.ip));
		s_retry_num = 0;
		xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
	}
}

esp_err_t wifi_init_sta(uint32_t timeout_ms)
{
	s_wifi_event_group = xEventGroupCreate();

	ESP_ERROR_CHECK(esp_netif_init());

	ESP_ERROR_CHECK(esp_event_loop_create_default());
	esp_netif_create_default_wifi_sta();

	wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
	ESP_ERROR_CHECK(esp_wifi_init(&cfg));
	ESP_ERROR_CHECK(esp_wifi_set_ps(WIFI_PS_NONE));
	//ESP_ERROR_CHECK(esp_wifi_set_bandwidth(WIFI_IF_STA, WIFI_BW_HT20));

	//esp_netif_t *sta_netif =
	//	esp_netif_get_handle_from_ifkey("WIFI_STA_DEF");
	//esp_netif_dhcpc_stop(sta_netif);

	//esp_netif_ip_info_t ip_info = {
	//	.ip = { .addr = ESP_IP4TOADDR(192, 168, 1, 211) },
	//	.gw = { .addr = ESP_IP4TOADDR(192, 168, 1, 254) },
	//	.netmask = { .addr = ESP_IP4TOADDR(255, 255, 255, 0) },
	//};
	//esp_netif_set_ip_info(sta_netif, &ip_info);

	//esp_netif_ip_info_t check;
	//esp_netif_get_ip_info(sta_netif, &check);
	//ESP_LOGI(TAG, "actual IP now: " IPSTR, IP2STR(&check.ip));

	esp_event_handler_instance_t instance_any_id;
	esp_event_handler_instance_t instance_got_ip;
	ESP_ERROR_CHECK(esp_event_handler_instance_register(
		WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL,
		&instance_any_id));
	ESP_ERROR_CHECK(esp_event_handler_instance_register(
		IP_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL,
		&instance_got_ip));

	wifi_config_t wifi_config = {
		.sta = { .ssid = CONFIG_APP_WIFI_SSID,
			 .password = CONFIG_APP_WIFI_PASS,
			 .threshold.authmode = WIFI_AUTH_WPA2_PSK },
	};
	ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
	ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
	ESP_ERROR_CHECK(esp_wifi_start());

	ESP_LOGI(TAG, "wifi_init_sta finished.");

	EventBits_t bits = xEventGroupWaitBits(
		s_wifi_event_group, WIFI_CONNECTED_BIT | WIFI_FAIL_BIT, pdFALSE,
		pdFALSE, pdMS_TO_TICKS(timeout_ms));

	if (bits & WIFI_CONNECTED_BIT) {
		ESP_LOGI(TAG, "connected to ap SSID:%s", CONFIG_APP_WIFI_SSID);
	} else if (bits & WIFI_FAIL_BIT) {
		ESP_LOGI(TAG, "Failed to connect to SSID:%s",
			 CONFIG_APP_WIFI_SSID);
		return ESP_FAIL;
	} else {
		ESP_LOGE(TAG, "Timed out waiting for Wi-Fi connection");
		esp_wifi_disconnect();
		return ESP_ERR_TIMEOUT;
	}

	return ESP_OK;
}
