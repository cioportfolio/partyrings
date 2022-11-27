#ifndef __COMMON__
#define __COMMON__

#include "settings.h"
#include <WiFi.h>
#include <WiFiMulti.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <ESPmDNS.h>

#define MAX_COMMANDS 5
enum action_t {screenBrightness, newStart, newAnalysis};
typedef struct {
  action_t action;
/*  uint8_t x;
  uint8_t y;
  uint8_t r;
  uint8_t g; */
  uint8_t b;
  uint32_t s;
} command_t ;

// volatile uint8_t score = 0;
// volatile uint8_t game_over = 1;

QueueHandle_t commandQ;
QueueHandle_t analysisQ;

/* QueueHandle_t frameQ;
QueueHandle_t scoreQ;
QueueHandle_t statusQ; */

#define NUM_LEDS (kMatrixWidth * kMatrixHeight)

typedef struct {
  uint8_t tempo;
  uint16_t barCount = 0;
  uint16_t beatCount = 0;
  uint16_t tatumCount = 0;
  uint16_t bars[MAX_BARS];
  uint16_t beats[MAX_BEATS];
  uint16_t tatums[MAX_TATUMS];
} analysis_t;

analysis_t analIn;
analysis_t analOut;

#endif
