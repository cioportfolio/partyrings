#include "common.h"
#include <FastLED.h>
#include "displayData.h"

uint8_t buff[kMatrixHeight][kMatrixWidth];
uint8_t blockX, blockY;
uint8_t blockT = 0;
uint8_t blockR = 0;
uint8_t blink_on = 1;
uint8_t blink_count = 0;
uint8_t step_speed = START_SPEED;
uint8_t step_count = 0;
uint8_t collapse = 0;
uint8_t coll_row_l, coll_row_h;
uint8_t drop = 0;
uint8_t flash = 0;

uint16_t XY(uint8_t x, uint8_t y)
{
  uint16_t i;
  uint8_t h = y;

  if (kMatrixTopToBottom == false)
  {
    h = kMatrixHeight - 1 - y;
  }

  if (kMatrixSerpentineLayout == false)
  {
    i = (h * kMatrixWidth) + x;
  }

  if (kMatrixSerpentineLayout == true)
  {
    if (y & 0x01)
    {
      // Even rows run backwards
      i = (h * kMatrixWidth) + x;
    }
    else
    {
      // Odd rows run forwards
      uint8_t reverseX = (kMatrixWidth - 1) - x;
      i = (h * kMatrixWidth) + reverseX;
    }
  }

  return i;
}

void next_block()
{
  blockT = random8(0, 6);
  blockR = random8(0, 4);
  blockY = 0;
  if (blocks[blockT][blockR][1] > 2)
    blockY = 1;

  uint8_t width = blocks[blockT][blockR][0];
  if (width == 1)
    blockX = random8(0, kMatrixWidth);
  else if (width == 2)
    blockX = random8(0, kMatrixWidth - 1);
  else if (width == 3)
    blockX = random(1, kMatrixWidth - 1);
  else
    blockX = random8(1, kMatrixWidth - 2);
  game_over = if_coll(blockX, blockY, blockR);
}


uint16_t XYsafe(uint8_t x, uint8_t y)
{
  if (x >= kMatrixWidth)
    return -1;
  if (y >= kMatrixHeight)
    return -1;
  return XY(x, y);
}

void genDisplay()
{
  for (uint8_t y = 0; y < kMatrixHeight; y++)
  {
    for (uint8_t x = 0; x < kMatrixWidth; x++)
    {
      if (collapse > 0 && y >= coll_row_l && y <= coll_row_h)
        leds[XYsafe(x, y)] = CHSV(0, 0, 90 * (1 + flash));
      else
        leds[XYsafe(x, y)] = CHSV(255 * y / kMatrixHeight, 255, buff[y][x]);
    }
  }
  if (collapse == 0)
  {
    for (uint8_t i = 0; i < blocks[blockT][blockR][2]; i++)
    {
      uint8_t p = blocks[blockT][blockR][3] + i;
      leds[XYsafe(blockX - 1 + pixels[p][0], blockY - 1 + pixels[p][1])] = CHSV(0, 0, 90 * (1 + blink_on));
    }
  }
}

TickType_t lastShow = xTaskGetTickCount();

void Refresh()
{
  const uint32_t delayTicks = 1000 / UPDATES_PER_SECOND / portTICK_PERIOD_MS;
  vTaskDelayUntil(&lastShow, delayTicks);
  FastLED.show();
  lastShow = xTaskGetTickCount();
}

uint8_t if_coll(uint8_t x, uint8_t y, uint8_t r)
{
  for (uint8_t i = 0; i < blocks[blockT][r][2]; i++)
  {
    uint8_t p = blocks[blockT][r][3] + i;

    if (y + pixels[p][1] > kMatrixHeight)
      return 1;
    if (x + pixels[p][0] < 1)
      return 1;
    if (x + pixels[p][0] > kMatrixWidth)
      return 1;
    if (buff[y - 1 + pixels[p][1]][x - 1 + pixels[p][0]] > 0)
      return 1;
  }
  return 0;
}

void collide()
{
  uint8_t spaces;
  uint8_t row_l = kMatrixHeight - 1;
  uint8_t row_h = 0;

  coll_row_l = kMatrixHeight - 1;
  coll_row_h = 0;

  for (uint8_t i = 0; i < blocks[blockT][blockR][2]; i++)
  {
    uint8_t p = blocks[blockT][blockR][3] + i;
    uint8_t y = blockY - 1 + pixels[p][1];

    if (y < row_l)
      row_l = y;
    if (y > row_h)
      row_h = y;
    buff[y][blockX - 1 + pixels[p][0]] = 255;
  }
  drop = 0;
  for (uint8_t y = row_l; y <= row_h; y++)
  {
    spaces = kMatrixWidth;
    for (uint8_t x = 0; x < kMatrixWidth; x++)
    {
      if (buff[y][x])
        spaces--;
    }
    if (spaces == 0)
    {
      collapse = UPDATES_PER_COLLAPSE;
      if (y < coll_row_l)
        coll_row_l = y;
      if (y > coll_row_h)
        coll_row_h = y;
    }
  }
  if (collapse == 0)
    next_block();
}

void drop_row()
{
  int8_t no_rows = coll_row_h - coll_row_l + 1;

  for (int8_t y = coll_row_h; y >= 0; y--)
  {
    for (uint8_t x = 0; x < kMatrixWidth; x++)
    {
      if (y > no_rows - 1)
        buff[y][x] = buff[y - no_rows][x];
      else
        buff[y][x] = 0;
    }
  }
  score++;
  if (step_speed > 1)
    step_speed--;
}

void reset_game()
{
  for (uint8_t y = 0; y < kMatrixHeight; y++)
  {
    for (uint8_t x = 0; x < kMatrixWidth; x++)
    {
      buff[y][x] = 0;
    }
  }
  score = 0;
  step_speed = START_SPEED;
  next_block();
}

void handle_controls()
{

  char command = ' ';

  while (uxQueueMessagesWaiting(commandQ) > 0) {
    xQueueReceive(commandQ, &command, portMAX_DELAY);
    Serial.print("Command : ");
    Serial.println(command);
    switch (command)
    {
      case 'L':
        if (blockX > 0)
          if (if_coll(blockX - 1, blockY, blockR) == 0)
            blockX--;
        break;

      case 'R':
        if (blockX + 1 < kMatrixWidth)
          if (if_coll(blockX + 1, blockY, blockR) == 0)
            blockX++;
        break;

      case 'l':
        /*      Serial.print("R : ");
              Serial.print(blockR);
              Serial.print(" R-1: ");
              Serial.print((blockR - 1) & 3);
              Serial.print("\n\n");*/

        if (if_coll(blockX, blockY, (blockR - 1) & 3) == 0)
          blockR = (blockR - 1) & 3;
        break;

      case 'r':
        /*      Serial.print("R : ");
              Serial.print(blockR);
              Serial.print(" R+1: ");
              Serial.print((blockR + 1) & 3);
              Serial.print("\n\n"); */
        if (if_coll(blockX, blockY, (blockR + 1) & 3) == 0)
          blockR = (blockR + 1) & 3;
        break;

      case 'D':
        drop = 1;
        break;

      case 'X':
        game_over = 1;
        break;

      case 'P':
        reset_game();
        game_over = 0;
        break;

      default:
        break;
    }
  }
}

void displayTask(void *params)
{

  Serial.print("Display task on core ");
  Serial.println(xPortGetCoreID());

  vTaskDelay(3000 / portTICK_PERIOD_MS); // power-up safety delay

  FastLED.addLeds<CHIPSET, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);

  for (;;)
  {
    random16_add_entropy(esp_random() & 0xFFFF);

    if (millis() < 5000)
    {
      FastLED.setBrightness(scale8(BRIGHTNESS, (millis() * 256) / 5000));
    }
    else
    {
      FastLED.setBrightness(BRIGHTNESS);
    }

    handle_controls();

    if (game_over == 0)
    {
      blink_count++;
      if (blink_count > UPDATES_PER_BLINK)
      {
        blink_on = 1 - blink_on;
        blink_count = 0;
      }
      if (collapse > 0)
      {
        collapse--;
        if (collapse == 0)
        {
          drop_row();
          next_block();
        }
      }
      else
      {
        step_count++;
        if (step_count > step_speed || drop == 1)
        {
          step_count = 0;
          if (if_coll(blockX, blockY + 1, blockR) == 1)
            collide();
          else
            blockY++;
        }
      }
    }
    flash = 1 - flash;
    genDisplay();
    Refresh();
  }
}
