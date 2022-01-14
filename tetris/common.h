#ifndef __COMMON__
#define __COMMON__

#include "settings.h"
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>

#define MAX_COMMANDS 5
enum action_t {moveLeft, moveRight, moveDown, rotateLeft, rotateRight, gamePlay, gameStop, paintPixel, screenFill, screenImage, screenBrightness};
typedef struct {
  action_t action;
  uint8_t x;
  uint8_t y;
  uint8_t r;
  uint8_t g;
  uint8_t b;
} command_t ;
volatile uint16_t score = 0;
volatile uint8_t game_over = 1;
QueueHandle_t commandQ;

#endif
