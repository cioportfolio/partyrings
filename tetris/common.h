#ifndef COMMON
#define COMMON common

#include "settings.h"
#define MAX_COMMANDS 5
#define BYTES_PER_COMMAND 1
volatile uint16_t score = 0;
volatile uint8_t game_over = 1;
QueueHandle_t commandQ;

#endif
