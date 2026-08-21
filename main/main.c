/* main.c
 *
 * Copyright (C) 2006-2026 wolfSSL Inc.
 *
 * This file is part of wolfSSL.
 *
 * wolfSSL is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * wolfSSL is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1335, USA
 */

/* FreeRTOS */
#include "logging.h"
#include <freertos/FreeRTOS.h>

/* Espressif */
#include <esp_event.h>
#include <esp_log.h>
#include <esp_netif.h>
#include <esp_netif_sntp.h>
#include <nvs_flash.h>

/* Sockets (lwIP) */
#include <lwip/netdb.h>
#include <lwip/sockets.h>
#include <lwip/dns.h>

/* wolfSSL */
/* Always include wolfcrypt/settings.h before any other wolfSSL file.    */
/* Reminder: settings.h pulls in user_settings.h; don't include it here. */
#ifdef WOLFSSL_USER_SETTINGS
#include <wolfssl/wolfcrypt/settings.h>
#ifndef WOLFSSL_ESPIDF
#warning "Problem with wolfSSL user_settings."
#warning "Check components/wolfssl/include"
#endif
#include <wolfssl/wolfcrypt/port/Espressif/esp32-crypt.h>
#else
/* Define WOLFSSL_USER_SETTINGS project wide for settings.h to include   */
/* wolfSSL user settings in ./components/wolfssl/include/user_settings.h */
#error "Missing WOLFSSL_USER_SETTINGS in CMakeLists or Makefile:\
    CFLAGS +=-DWOLFSSL_USER_SETTINGS"
#endif

#include <wolfssl/ssl.h>

/* project */
#include "wifi.h"
#include "rootCA.h"
#include "main.h"
#include "config.h"

//--------------------- DEFINES ---------------------//
#define END_MARKER "END MAIN"
//--------------------- GLOBAL VARIABLES ---------------------//

static const char *const TAG = "tls_client";
#ifdef KEY_EXCHANGE_X25519
static const int KEY_EXCHANGE_GROUPS[] = {
	WOLFSSL_ECC_X25519,
};
#elif defined(KEY_EXCHANGE_ML_KEM_512)
static const int KEY_EXCHANGE_GROUPS[] = {
	WOLFSSL_ML_KEM_512,
};
#else
#error "Failed to read KEY_EXCHANGE_X"
#endif
static int KEY_EXCHANGE_GROUPS_SZ = 1;

//--------------------- FUNCTION'S DECLARATIONS ---------------------//

//static void set_dns_server(void);
static void sync_time(void);
static void run_tls_client(int repeats);
static int graceful_shutdown(WOLFSSL *ssl);

//--------------------- MAIN FUNCTION ---------------------//

void app_main(void)
{
#ifdef DEBUG_WOLFSSL
	wolfSSL_Debugging_ON();
	ESP_LOGW(TAG, "DEBUGGING TURN ON");
#endif
	ESP_ERROR_CHECK(nvs_flash_init());
	ESP_ERROR_CHECK(wifi_init_sta(190000));
	sync_time();
	run_tls_client(CONNECTION_REPEATS);

	ESP_LOGI(TAG, END_MARKER);
}
//--------------------- FUNCTION'S IMPLEMENTAIONS ---------------------//
static void sync_time(void)
{
	esp_sntp_config_t config =
		ESP_NETIF_SNTP_DEFAULT_CONFIG("pool.ntp.org");

	ESP_LOGI(TAG, "Syncing time via SNTP ...");
	esp_netif_sntp_init(&config);
	if (esp_netif_sntp_sync_wait(pdMS_TO_TICKS(60000)) != ESP_OK) {
		ESP_LOGW(
			TAG,
			"SNTP sync timed out; certificate date check may fail");
	}
}

static void run_tls_client(int repeats)
{
	const char *host = CONFIG_APP_TARGET_HOST;
	const uint16_t port = (uint16_t)CONFIG_APP_TARGET_PORT;
	struct sockaddr_in servAddr;
	struct hostent *hp;
	int sockfd;
	int ret;
	WOLFSSL_CTX *ctx;
	WOLFSSL *ssl;

	ESP_LOGI(TAG, "Resolving %s ...", host);
	hp = gethostbyname(host);
	if (hp == NULL) {
		ESP_LOGE(TAG, "DNS lookup failed for %s", host);
		return;
	}

	wolfSSL_Init();

	ctx = wolfSSL_CTX_new(wolfTLSv1_3_client_method());
	if (ctx == NULL) {
		ESP_LOGE(TAG, "wolfSSL_CTX_new() failed");
		return;
	}

	ret = wolfSSL_CTX_load_verify_buffer(ctx,
					     (const unsigned char *)rootCA_cert,
					     (long)sizeof(rootCA_cert),
					     WOLFSSL_FILETYPE_PEM);
	if (ret != WOLFSSL_SUCCESS) {
		ESP_LOGE(TAG, "wolfSSL_CTX_load_verify_buffer() failed: %d",
			 ret);
		wolfSSL_CTX_free(ctx);
		return;
	}
	wolfSSL_CTX_set_verify(ctx, WOLFSSL_VERIFY_PEER, NULL);
	if (wolfSSL_CTX_set_groups(ctx, (int *)KEY_EXCHANGE_GROUPS,
				   KEY_EXCHANGE_GROUPS_SZ) != WOLFSSL_SUCCESS) {
		ESP_LOGE(TAG, "wolfSSL_CTX_set_groups() failed: %d", ret);
		wolfSSL_CTX_free(ctx);
		return;
	}

	for (int i = 0; i < repeats; i++) {
		sockfd = socket(AF_INET, SOCK_STREAM, 0);
		if (sockfd < 0) {
			ESP_LOGE(TAG, "socket() failed: errno %d", errno);
			return;
		}

		memset(&servAddr, 0, sizeof(servAddr));
		servAddr.sin_family = AF_INET;
		servAddr.sin_port = htons(port);
		memcpy(&servAddr.sin_addr.s_addr, hp->h_addr, hp->h_length);

		ESP_LOGI(TAG, "Opening TCP connection to %s:%d ...", host,
			 port);
		if (connect(sockfd, (struct sockaddr *)&servAddr,
			    sizeof(servAddr)) != 0) {
			ESP_LOGE(TAG, "TCP connect() failed: errno %d", errno);
			close(sockfd);
			return;
		}
		ESP_LOGI(TAG, "TCP connected.");
		ssl = wolfSSL_new(ctx);
		if (ssl == NULL) {
			ESP_LOGE(TAG, "wolfSSL_new() failed");
			wolfSSL_CTX_free(ctx);
			close(sockfd);
			return;
		}
		wolfSSL_set_fd(ssl, sockfd);
		wolfSSL_check_domain_name(ssl, host);

		ESP_LOGI(TAG, "TLS handshake starting...");
		ret = wolfSSL_connect(ssl);
		if (ret != WOLFSSL_SUCCESS) {
			ESP_LOGE(TAG, "wolfSSL_connect() failed: %d",
				 wolfSSL_get_error(ssl, ret));
			wolfSSL_free(ssl);
			wolfSSL_CTX_free(ctx);
			close(sockfd);
			return;
		}
		ESP_LOGI(TAG, "Handshake complete: %s, cipher %s",
			 wolfSSL_get_version(ssl),
			 wolfSSL_get_cipher_name(ssl));

		graceful_shutdown(ssl);
		wolfSSL_free(ssl);
		close(sockfd);
	}

	wolfSSL_CTX_free(ctx);
	wolfSSL_Cleanup();
}

static int graceful_shutdown(WOLFSSL *ssl)
{
	int ret = wolfSSL_shutdown(ssl); /* pisze close_notify, wraca od razu */

	if (ret >= 0 && ret != WOLFSSL_SUCCESS)
		ret = wolfSSL_shutdown(ssl); /* czyta, blokuje do odpowiedzi */

	if (ret != WOLFSSL_SUCCESS) {
		int err = wolfSSL_get_error(ssl, ret);
		ESP_LOGE(TAG, "shutdown niepełny: ret=%d err=%d (%s)", ret, err,
			 wolfSSL_ERR_reason_error_string(err));
		return err;
	}
	return WOLFSSL_SUCCESS;
}
