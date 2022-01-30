#ifndef __SETTINGS__
#define __SETTINGS__

#define BRIGHTNESS 64
#define LED_PIN 5
#define COLOR_ORDER GRB
#define CHIPSET WS2812B

#define SW_PIN 21
#define X_PIN 35
#define Y_PIN 32
#define JOY_SENS 250
#define JOY_REF 1900
#define JOY_LO (JOY_REF-JOY_SENS)
#define JOY_HI (JOY_REF+JOY_SENS)

#define UPDATES_PER_SECOND 25
#define UPDATES_PER_BLINK 12
#define UPDATES_PER_COLLAPSE 25
#define START_SPEED 30

const char* ssid = "techjam";

// Params for width and height
const uint8_t kMatrixWidth = 15;
const uint8_t kMatrixHeight = 15;

// Param for different pixel layouts
const bool kMatrixSerpentineLayout = true;
const bool kMatrixTopToBottom = false;

#endif
