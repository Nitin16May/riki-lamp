#pragma once

#define DEVICE_NAME "RiKi Lamp"
#define FIRMWARE_VERSION "1.0.0"

#define AP_SSID "RiKiLamp-Setup"
#define AP_IP "192.168.4.1"

#define WIFI_CONNECT_TIMEOUT_MS 15000
#define WIFI_RECONNECT_INTERVAL_MS 10000

// --- Hardware (ESP32-C3 Super Mini) ---
#define LED_PIN 8
#define LED_COUNT 24
#define LED_TYPE WS2812B
#define LED_COLOR_ORDER GRB

#define TOUCH_ACTIVE HIGH
#define TOUCH_PIN_1 1
#define TOUCH_PIN_2 2
#define TOUCH_PIN_3 3
#define TOUCH_PIN_4 4

// --- Touch timing ---
#define TOUCH_LONG_PRESS_MS 2000
#define TOUCH_FACTORY_RESET_MS 8000
#define TOUCH_DOUBLE_TAP_MS 400
#define TOUCH_MULTI_TAP_TIMEOUT_MS 500
#define TOUCH_DEBOUNCE_MS 50

// --- Diagnostics ---
#define DIAGNOSTICS_INTERVAL_MS 30000

// --- Effects ---
#define LED_FPS_MS 33
