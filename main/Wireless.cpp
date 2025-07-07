

#include "Wireless.h"
#include <cstdio>
#include <cstddef>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_wifi.h"


bool WIFI_Connection = 0;
uint8_t WIFI_NUM = 0;
uint8_t BLE_NUM = 0;
bool Scan_finish = 0;

int wifi_scan_number()
{
    printf("/**********WiFi Test**********/\r\n");
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&cfg);
    esp_wifi_set_mode(WIFI_MODE_STA);
    esp_wifi_start();

    wifi_scan_config_t scan_config = {0};
    esp_wifi_scan_start(&scan_config, true);

    uint16_t ap_num = 0;
    esp_wifi_scan_get_ap_num(&ap_num);
    wifi_ap_record_t *ap_records = (wifi_ap_record_t *)malloc(sizeof(wifi_ap_record_t) * ap_num);
    esp_wifi_scan_get_ap_records(&ap_num, ap_records);

    if (ap_num == 0) {
        printf("No WIFI device was scanned\r\n");
    } else {
        printf("Scanned %d Wi-Fi devices\r\n", ap_num);
    }

    free(ap_records);
    esp_wifi_stop();
    printf("/*******WiFi Test Over********/\r\n\r\n");
    return ap_num;
}
// BLE scan stub for ESP-IDF (real implementation requires NimBLE or Bluedroid APIs)
int ble_scan_number()
{
    printf("/**********BLE Test**********/\r\n");
    printf("BLE scan not implemented in this stub.\r\n");
    printf("/**********BLE Test Over**********/\r\n\r\n");
    return 0;
}
extern char buffer[128];    /* Make sure buffer is enough for `sprintf` */
void Wireless_Test1(){
  // BLE_NUM = ble_scan_number();                       // !!! Please note that continuing to use Bluetooth will result in allocation failure due to RAM usage, so pay attention to RAM usage when Bluetooth is turned on
  WIFI_NUM = wifi_scan_number();
  Scan_finish = 1;
}

void WirelessScanTask(void *parameter) {
  // BLE_NUM = ble_scan_number();                       // !!! Please note that continuing to use Bluetooth will result in allocation failure due to RAM usage, so pay attention to RAM usage when Bluetooth is turned on
  WIFI_NUM = wifi_scan_number();
  Scan_finish = 1;
  vTaskDelay(pdMS_TO_TICKS(1000));
  vTaskDelete(NULL);
}
void Wireless_Test2(){
  xTaskCreatePinnedToCore(
    WirelessScanTask,    
    "WirelessScanTask",   
    4096,                
    NULL,                 
    2,                   
    NULL,                 
    0                   
  );
}
