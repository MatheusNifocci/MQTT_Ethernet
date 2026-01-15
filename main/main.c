#include <stdio.h>
#include "esp_eth_phy.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_netif.h"
#include "esp_eth.h"
#include "driver/gpio.h"
#include "esp_eth_mac.h"
#include "lwip/ip4_addr.h"

#include "mqtt_client.h"

static const char *TAG = "ETH_APP";


// MQTT

static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data) {
	
    esp_mqtt_event_handle_t event = event_data;

    switch (event_id) {

        case MQTT_EVENT_CONNECTED:
            ESP_LOGI("MQTT", "Conectado ao broker");
            esp_mqtt_client_subscribe(event->client, "esp32/teste", 0);
            
            esp_mqtt_client_publish(event->client,  "esp32/status",  "ESP32 online",  0, 1, 0);
            
            break;

        case MQTT_EVENT_DISCONNECTED:
            ESP_LOGW("MQTT", "Desconectado do broker");
            break;

        case MQTT_EVENT_DATA:
            ESP_LOGI("MQTT", "Msg recebida:");
            ESP_LOGI("MQTT", "TOPIC=%.*s", event->topic_len, event->topic);
            ESP_LOGI("MQTT", "DATA=%.*s", event->data_len, event->data);
            break;

        default:
            break;
    }
}

static void mqtt_start(void) {
	
    esp_mqtt_client_config_t mqtt_cfg = {
        .broker.address.uri = "mqtt://192.168.10.1",  // IP do broker
        .credentials.client_id = "esp32_eth_01",
    };

    esp_mqtt_client_handle_t client = esp_mqtt_client_init(&mqtt_cfg);

    esp_mqtt_client_register_event(client,
                                   ESP_EVENT_ANY_ID,
                                   mqtt_event_handler,
                                   NULL);

    esp_mqtt_client_start(client);
}


//ETHERNET

static void set_static_ip(esp_netif_t *netif) {
    esp_netif_ip_info_t ip_info;

    IP4_ADDR(&ip_info.ip,      192, 168, 10, 50);
    IP4_ADDR(&ip_info.gw,      192, 168, 10, 1);
    IP4_ADDR(&ip_info.netmask, 255, 255, 255, 0);

    ESP_ERROR_CHECK(esp_netif_dhcpc_stop(netif));
    ESP_ERROR_CHECK(esp_netif_set_ip_info(netif, &ip_info));

    ESP_LOGI("ETH", "IP fixo configurado: " IPSTR,
             IP2STR(&ip_info.ip));
}



static void eth_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data) {
	
    switch (event_id) {
        case ETHERNET_EVENT_CONNECTED:
            ESP_LOGI("ETH", "Ethernet Link Up");
            break;
        case ETHERNET_EVENT_DISCONNECTED:
            ESP_LOGI("ETH", "Ethernet Link Down");
            break;
        case ETHERNET_EVENT_START:
            ESP_LOGI("ETH", "Ethernet Started");
            break;
        case ETHERNET_EVENT_STOP:
            ESP_LOGI("ETH", "Ethernet Stopped");
            break;
        default:
            break;
    }
}

static void got_ip_event_handler(void *arg, esp_event_base_t event_base,  int32_t event_id, void *event_data) {
	
    ip_event_got_ip_t *event = (ip_event_got_ip_t *) event_data;
    ESP_LOGI("ETH", "Got IP: " IPSTR, IP2STR(&event->ip_info.ip));
    
    mqtt_start();
}




void app_main(void)
{
    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    // Cria interface Ethernet
    esp_netif_config_t netif_cfg = ESP_NETIF_DEFAULT_ETH();
    esp_netif_t *eth_netif = esp_netif_new(&netif_cfg);

	// Configuração MAC genérica
	eth_mac_config_t mac_config = ETH_MAC_DEFAULT_CONFIG();
	
	// Configuração específica do EMAC do ESP32
	eth_esp32_emac_config_t esp32_emac_config = ETH_ESP32_EMAC_DEFAULT_CONFIG();
	
	esp32_emac_config.clock_config.rmii.clock_mode = EMAC_CLK_EXT_IN;
    esp32_emac_config.clock_config.rmii.clock_gpio = 0;   // GPIO0 (ou 16)
	
    esp_eth_mac_t *mac = esp_eth_mac_new_esp32(&esp32_emac_config, &mac_config);

    // Configuração PHY
    eth_phy_config_t phy_config = ETH_PHY_DEFAULT_CONFIG();
    phy_config.phy_addr = 1;
    phy_config.reset_gpio_num = 5;

    esp_eth_phy_t *phy = esp_eth_phy_new_lan87xx(&phy_config);

    // Driver Ethernet
    esp_eth_config_t eth_config = ETH_DEFAULT_CONFIG(mac, phy);
    esp_eth_handle_t eth_handle = NULL;

    ESP_ERROR_CHECK(esp_eth_driver_install(&eth_config, &eth_handle));
   
   	esp_eth_netif_glue_handle_t eth_glue = esp_eth_new_netif_glue(eth_handle);
	ESP_ERROR_CHECK(esp_netif_attach(eth_netif, eth_glue));
	

  
    ESP_ERROR_CHECK(esp_event_handler_register(ETH_EVENT, ESP_EVENT_ANY_ID, &eth_event_handler, NULL));
	ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_ETH_GOT_IP, &got_ip_event_handler, NULL));


	set_static_ip(eth_netif); 
	
    ESP_ERROR_CHECK(esp_eth_start(eth_handle));

    ESP_LOGI(TAG, "Ethernet inicializado");
}


