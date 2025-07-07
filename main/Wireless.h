#pragma once
#include <stdint.h>
#include <stdbool.h>

// Forward declarations for ESP-IDF Wi-Fi and BLE functionality will be in the .cpp file

extern bool WIFI_Connection;
extern uint8_t WIFI_NUM;
extern uint8_t BLE_NUM;
extern bool Scan_finish;

int wifi_scan_number(void);
int ble_scan_number(void);
void Wireless_Test1(void);
void Wireless_Test2(void);