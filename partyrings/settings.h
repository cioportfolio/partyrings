#ifndef __SETTINGS__
#define __SETTINGS__

#define BRIGHTNESS 32
#define LED_PIN 22
#define COLOR_ORDER GRB
#define CHIPSET WS2812B

#ifndef LED_BUILTIN
#define LED_BUILTIN 5
#endif

/* #define SW_PIN 21
#define X_PIN 35
#define Y_PIN 32
#define JOY_SENS 250
#define JOY_REF 1900
#define JOY_LO (JOY_REF-JOY_SENS)
#define JOY_HI (JOY_REF+JOY_SENS) */

#define UPDATES_PER_SECOND 50
/* #define UPDATES_PER_BLINK 12
#define UPDATES_PER_COLLAPSE 25
#define START_SPEED 30 */

#define URL "http://192.168.1.20:8001" //office pc

// Params for width and height
const uint8_t kMatrixWidth = 1;
const uint8_t kMatrixHeight = 24;

// Param for different pixel layouts
//const bool kMatrixSerpentineLayout = true;
//const bool kMatrixTopToBottom = false;

#define MAX_BARS 300
#define MAX_BEATS 1200
#define MAX_TATUMS 2400

#endif
