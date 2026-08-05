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
#include <freertos/FreeRTOS.h>

/* Espressif */
#include <esp_log.h>
#include <nvs_flash.h>
#include <esp_netif.h>
#include <esp_netif_sntp.h>
#include <esp_event.h>
#include <protocol_examples_common.h>

/* Sockets (lwIP) */
#include <lwip/netdb.h>
#include <lwip/sockets.h>

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
#include "main.h"
#include "ca_cert.h"

static const char* const TAG = "tls_client";

/* Certificate validity dates are checked against the system clock, and the
 * ESP32 has no battery-backed RTC - it boots at/near the epoch. Without this,
 * wolfSSL_connect() below would fail with a "certificate expired/not yet
 * valid" error even with a correct CA loaded. */
static void sync_time(void)
{
    esp_sntp_config_t config = ESP_NETIF_SNTP_DEFAULT_CONFIG("pool.ntp.org");

    ESP_LOGI(TAG, "Syncing time via SNTP ...");
    esp_netif_sntp_init(&config);
    if (esp_netif_sntp_sync_wait(pdMS_TO_TICKS(10000)) != ESP_OK) {
        ESP_LOGW(TAG, "SNTP sync timed out; certificate date check may fail");
    }
}

/* Step 2 + 3 + 4: plain TCP socket, then the TLS 1.3 handshake on top of it,
 * then a simple HTTP GET through the encrypted tunnel. */
static void run_tls_client(void)
{
    const char* host = CONFIG_WOLFSSL_TARGET_HOST;
    const uint16_t port = (uint16_t)CONFIG_WOLFSSL_TARGET_PORT;
    struct sockaddr_in servAddr;
    struct hostent* hp;
    char buf[512];
    int sockfd;
    int ret;
    WOLFSSL_CTX* ctx;
    WOLFSSL* ssl;

    ESP_LOGI(TAG, "Resolving %s ...", host);
    hp = gethostbyname(host);
    if (hp == NULL) {
        ESP_LOGE(TAG, "DNS lookup failed for %s", host);
        return;
    }

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        ESP_LOGE(TAG, "socket() failed: errno %d", errno);
        return;
    }

    memset(&servAddr, 0, sizeof(servAddr));
    servAddr.sin_family = AF_INET;
    servAddr.sin_port = htons(port);
    memcpy(&servAddr.sin_addr.s_addr, hp->h_addr, hp->h_length);

    ESP_LOGI(TAG, "Opening TCP connection to %s:%d ...", host, port);
    if (connect(sockfd, (struct sockaddr*)&servAddr, sizeof(servAddr)) != 0) {
        ESP_LOGE(TAG, "TCP connect() failed: errno %d", errno);
        close(sockfd);
        return;
    }
    ESP_LOGI(TAG, "TCP connected. Starting TLS 1.3 handshake ...");

    wolfSSL_Init();

    ctx = wolfSSL_CTX_new(wolfTLSv1_3_client_method());
    if (ctx == NULL) {
        ESP_LOGE(TAG, "wolfSSL_CTX_new() failed");
        close(sockfd);
        return;
    }

    ret = wolfSSL_CTX_load_verify_buffer(ctx,
                    (const unsigned char*)globalsign_root_r3_pem,
                    (long)sizeof(globalsign_root_r3_pem),
                    WOLFSSL_FILETYPE_PEM);
    if (ret != WOLFSSL_SUCCESS) {
        ESP_LOGE(TAG, "wolfSSL_CTX_load_verify_buffer() failed: %d", ret);
        wolfSSL_CTX_free(ctx);
        close(sockfd);
        return;
    }
    wolfSSL_CTX_set_verify(ctx, WOLFSSL_VERIFY_PEER, NULL);

    ssl = wolfSSL_new(ctx);
    if (ssl == NULL) {
        ESP_LOGE(TAG, "wolfSSL_new() failed");
        wolfSSL_CTX_free(ctx);
        close(sockfd);
        return;
    }
    wolfSSL_set_fd(ssl, sockfd);

    /* Cloudflare (which fronts example.com) hosts many sites on shared IPs
     * and needs SNI in the ClientHello to know which cert to present.
     * Without it, it sends a fatal "handshake failure" alert. */
    wolfSSL_UseSNI(ssl, WOLFSSL_SNI_HOST_NAME, host, (word16)strlen(host));

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
              wolfSSL_get_version(ssl), wolfSSL_get_cipher_name(ssl));

    ret = snprintf(buf, sizeof(buf),
                    "GET / HTTP/1.1\r\nHost: %s\r\nConnection: close\r\n\r\n",
                    host);
    wolfSSL_write(ssl, buf, ret);

    ESP_LOGI(TAG, "Reading response ...");
    do {
        ret = wolfSSL_read(ssl, buf, sizeof(buf) - 1);
        if (ret > 0) {
            buf[ret] = '\0';
            ESP_LOGI(TAG, "%d bytes:\n%s", ret, buf);
        }
    } while (ret > 0);

    wolfSSL_shutdown(ssl);
    wolfSSL_free(ssl);
    wolfSSL_CTX_free(ctx);
    wolfSSL_Cleanup();
    close(sockfd);
}

void app_main(void)
{
    /* Step 1: join WiFi and get an IP address. NVS is required by the
     * WiFi driver to store calibration data. */
    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    ESP_ERROR_CHECK(example_connect());

    sync_time();
    run_tls_client();

    ESP_LOGI(TAG, "\n\nDone!"
                  "If running from idf.py monitor, press twice: Ctrl+]\n\n"
                  "WOLFSSL_COMPLETE\n" /* exit keyword for wolfssl_monitor.py */
            );
}
