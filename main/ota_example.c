\
    #include <string.h>
    #include "freertos/FreeRTOS.h"
    #include "freertos/event_groups.h"
    #include "freertos/task.h"
    #include "esp_log.h"
    #include "esp_system.h"
    #include "nvs_flash.h"
    #include "esp_wifi.h"
    #include "esp_event.h"
    #include "esp_https_ota.h"
    #include "esp_app_desc.h"
    #include "esp_ota_ops.h"

    #include "secrets.h"

    static const char *TAG = "OTA_DEMO";
    static EventGroupHandle_t wifi_event_group;
    const int WIFI_CONNECTED_BIT = BIT0;

    /** Wi-Fi event handler */
    static void wifi_event_handler(void* arg, esp_event_base_t event_base,
                                   int32_t event_id, void* event_data)
    {
        if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
            esp_wifi_connect();
        } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
            esp_wifi_connect();
            ESP_LOGI(TAG, "Retrying Wi‑Fi connection");
        } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
            xEventGroupSetBits(wifi_event_group, WIFI_CONNECTED_BIT);
        }
    }

    static void wifi_init(void)
    {
        wifi_event_group = xEventGroupCreate();
        ESP_ERROR_CHECK(esp_netif_init());
        ESP_ERROR_CHECK(esp_event_loop_create_default());
        esp_netif_create_default_wifi_sta();

        wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
        ESP_ERROR_CHECK(esp_wifi_init(&cfg));
        ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL, NULL));
        ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_event_handler, NULL, NULL));

        wifi_config_t wifi_config = {0};
        strcpy((char*)wifi_config.sta.ssid, WIFI_SSID);
        strcpy((char*)wifi_config.sta.password, WIFI_PASS);
        wifi_config.sta.threshold.authmode = WIFI_AUTH_WPA2_PSK;
        wifi_config.sta.pmf_cfg.capable = true;
        wifi_config.sta.pmf_cfg.required = false;

        ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
        ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
        ESP_ERROR_CHECK(esp_wifi_start());

        ESP_LOGI(TAG, "wifi_init: finished");
        /* Wait for connection */
        xEventGroupWaitBits(wifi_event_group, WIFI_CONNECTED_BIT, pdFALSE, pdTRUE, portMAX_DELAY);
    }

    static void try_ota_update(void)
    {
        ESP_LOGI(TAG, "Checking for new firmware at %s", OTA_URL);
        esp_http_client_config_t config = {
            .url = OTA_URL,
            .timeout_ms = 10000,
            .cert_pem = NULL,   // For quick test with HTTP; supply server cert for HTTPS
        };
        esp_err_t ret = esp_https_ota(&config);
        if (ret == ESP_OK) {
            ESP_LOGI(TAG, "OTA successful, restarting...");
            esp_restart();
        } else if (ret == ESP_ERR_OTA_UPGRADE_DONE) {
            ESP_LOGI(TAG, "No new firmware.");
        } else {
            ESP_LOGE(TAG, "OTA failed: %s", esp_err_to_name(ret));
        }
    }

    void app_main(void)
    {
        ESP_LOGI(TAG, "Booting firmware build %s", APP_VERSION);
        ESP_ERROR_CHECK(nvs_flash_init());
        wifi_init();
        try_ota_update();

        while (1) {
            ESP_LOGI(TAG, "Running...");
            vTaskDelay(pdMS_TO_TICKS(5000));
        }
    }
