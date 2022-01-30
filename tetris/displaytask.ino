#include "common.h"
#include <FastLED.h>
#include "displayData.h"

#define GAMEWIDTH 10

uint8_t buff[kMatrixHeight][kMatrixWidth];
uint8_t blockX[2], blockY[2];
uint8_t blockT[2] = {0, 0};
uint8_t blockR[2] = {0, 0};
uint8_t blink_on = 1;
uint8_t blink_count = 0;
uint8_t step_speed = START_SPEED;
uint8_t step_count = 0;
uint8_t collapse = 0;
uint8_t coll_row_l, coll_row_h;
uint8_t drop = 0;
uint8_t flash = 0;
uint8_t brightness = BRIGHTNESS;
uint8_t nextBlock = 0;

uint8_t pressX = 1;
uint8_t pressY = 1;
uint8_t pressS = 0;

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
  blockT[nextBlock] = random8(0, 6);
  blockR[nextBlock] = random8(0, 4);
  blockY[nextBlock] = 0;
  if (blocks[blockT[nextBlock]][blockR[nextBlock]][1] > 2)
    blockY[nextBlock] = 1;

  /*uint8_t width = blocks[blockT][blockR][0];
    if (width == 1)
    blockX = random8(0, kMatrixWidth);
    else if (width == 2)
    blockX = random8(0, kMatrixWidth - 1);
    else if (width == 3)
    blockX = random(1, kMatrixWidth - 1);
    else
    blockX = random8(1, kMatrixWidth - 2); */
  blockX[nextBlock] = 5;
  nextBlock = 1 - nextBlock;
  game_over = if_coll(blockX[nextBlock], blockY[nextBlock], blockR[nextBlock]);
  if (game_over == 1)
  {
    char c = 'O';
    xQueueSend(statusQ, &c, portMAX_DELAY);
  }
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
      else if (x == GAMEWIDTH)
        leds[XYsafe(GAMEWIDTH, y)] = CHSV(0, 0, 32);
      else
        leds[XYsafe(x, y)] = CHSV(255 * y / kMatrixHeight, 255, buff[y][x]);
    }
  }
  if (collapse == 0)
  {
    for (uint8_t i = 0; i < blocks[blockT[nextBlock]][blockR[nextBlock]][2]; i++)
    {
      uint8_t p = blocks[blockT[nextBlock]][blockR[nextBlock]][3] + i;
      leds[XYsafe(blockX[nextBlock] - 1 + pixels[p][0], blockY[nextBlock] - 1 + pixels[p][1])] = CRGB(blockCol[blockT[nextBlock]][0] / (1 + 2 * blink_on), blockCol[blockT[nextBlock]][1] / (1 + 2 * blink_on), blockCol[blockT[nextBlock]][2] / (1 + 2 * blink_on));
    }
  }
  for (uint8_t i = 0; i < blocks[blockT[1 - nextBlock]][blockR[1 - nextBlock]][2]; i++)
  {
    uint8_t p = blocks[blockT[1 - nextBlock]][blockR[1 - nextBlock]][3] + i;
    leds[XYsafe(GAMEWIDTH + 1 + pixels[p][0], 4 + pixels[p][1])] = CRGB(blockCol[blockT[1 - nextBlock]][0], blockCol[blockT[1 - nextBlock]][1], blockCol[blockT[1 - nextBlock]][2]);
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
  for (uint8_t i = 0; i < blocks[blockT[nextBlock]][r][2]; i++)
  {
    uint8_t p = blocks[blockT[nextBlock]][r][3] + i;

    if (y + pixels[p][1] > kMatrixHeight)
      return 1;
    if (x + pixels[p][0] < 1)
      return 1;
    if (x + pixels[p][0] > GAMEWIDTH)
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

  for (uint8_t i = 0; i < blocks[blockT[nextBlock]][blockR[nextBlock]][2]; i++)
  {
    uint8_t p = blocks[blockT[nextBlock]][blockR[nextBlock]][3] + i;
    uint8_t y = blockY[nextBlock] - 1 + pixels[p][1];

    if (y < row_l)
      row_l = y;
    if (y > row_h)
      row_h = y;
    buff[y][blockX[nextBlock] - 1 + pixels[p][0]] = 255;
  }
  drop = 0;
  for (uint8_t y = row_l; y <= row_h; y++)
  {
    spaces = GAMEWIDTH;
    for (uint8_t x = 0; x < GAMEWIDTH; x++)
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
  uint8_t s = score;
  xQueueSend(scoreQ, &s, portMAX_DELAY);
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
  uint8_t s = score;
  xQueueSend(scoreQ, &s, portMAX_DELAY);
  step_speed = START_SPEED;
  blockT[nextBlock] = random8(0, 6);
  blockR[nextBlock] = random8(0, 4);
  blockY[nextBlock] = 0;
  if (blocks[blockT[nextBlock]][blockR[nextBlock]][1] > 2)
    blockY[nextBlock] = 1;
  blockX[nextBlock] = 5;
  nextBlock = 1 - nextBlock;
  next_block();
}

void printCommand(command_t *c)
{
  Serial.print("Command :");
  Serial.print(c->action);
  Serial.print(" x=");
  Serial.print(c->x);
  Serial.print(" y=");
  Serial.print(c->y);
  Serial.print(" r=");
  Serial.print(c->r);
  Serial.print(" g=");
  Serial.print(c->g);
  Serial.print(" b=");
  Serial.println(c->b);
}

void tryMoveLeft()
{
  if (blockX[nextBlock] > 0)
    if (if_coll(blockX[nextBlock] - 1, blockY[nextBlock], blockR[nextBlock]) == 0)
      blockX[nextBlock]--;
}

void tryMoveRight()
{
  if (blockX[nextBlock] + 1 < GAMEWIDTH)
    if (if_coll(blockX[nextBlock] + 1, blockY[nextBlock], blockR[nextBlock]) == 0)
      blockX[nextBlock]++;
}

void tryRotateLeft()
{
  if (if_coll(blockX[nextBlock], blockY[nextBlock], (blockR[nextBlock] - 1) & 3) == 0)
    blockR[nextBlock] = (blockR[nextBlock] - 1) & 3;
}

void tryRotateRight()
{
  if (if_coll(blockX[nextBlock], blockY[nextBlock], (blockR[nextBlock] + 1) & 3) == 0)
    blockR[nextBlock] = (blockR[nextBlock] + 1) & 3;
}

void checkJoy()
{
  uint16_t joyX, joyY;

  joyX = analogRead(X_PIN);
  joyY = analogRead(Y_PIN);

  /*Serial.print(joyX);
    Serial.print("\t");
    Serial.println(joyY); */

  if (digitalRead(SW_PIN) == 0)
  {
    if (pressS == 0)
    {
      pressS = 1;
      Serial.println("Joystick : Switch");
      if (game_over == 1)
      {
        reset_game();
        char c = 'P';
        game_over = 0;
        xQueueSend(statusQ, &c, portMAX_DELAY);
      }
      else
        drop = 1;
    }
  }
  else
    pressS = 0;

  if (joyX < JOY_LO)
  {
    if (pressX > 0)
    {
      pressX = 0;
      Serial.println("Joystick : Right");
      tryMoveRight();
    }
  }
  else if (joyX > JOY_HI)
  {
    if (pressX < 2)
    {
      Serial.println("Joystick : Left");
      pressX = 2;
      tryMoveLeft();
    }
  }
  else
    pressX = 1;

  if (joyY < JOY_LO)
  {
    if (pressY > 0)
    {
      pressY = 0;
      Serial.println("Joystick : Up");
      tryRotateLeft();
    }
  }
  else if (joyY > JOY_HI)
  {
    if (pressY < 2)
    {
      pressY = 2;
      Serial.println("Joystick : Down");
      tryRotateRight();
    }
  }
  else
    pressY = 1;
}

void handle_controls()
{

  command_t command;
  uint8_t repaint = 0;

  checkJoy();

  while (uxQueueMessagesWaiting(commandQ) > 0)
  {
    char c;
    xQueueReceive(commandQ, &command, portMAX_DELAY);
    printCommand(&command);
    switch (command.action)
    {
      case moveLeft:
        if (blockX[nextBlock] > 0)
          if (if_coll(blockX[nextBlock] - 1, blockY[nextBlock], blockR[nextBlock]) == 0)
            blockX[nextBlock]--;
        break;

      case moveRight:
        if (blockX[nextBlock] + 1 < GAMEWIDTH)
          if (if_coll(blockX[nextBlock] + 1, blockY[nextBlock], blockR[nextBlock]) == 0)
            blockX[nextBlock]++;
        break;

      case rotateLeft:
        /*      Serial.print("R : ");
                Serial.print(blockR);
                Serial.print(" R-1: ");
                Serial.print((blockR - 1) & 3);
                Serial.print("\n\n");*/

        if (if_coll(blockX[nextBlock], blockY[nextBlock], (blockR[nextBlock] - 1) & 3) == 0)
          blockR[nextBlock] = (blockR[nextBlock] - 1) & 3;
        break;

      case rotateRight:
        /*      Serial.print("R : ");
                Serial.print(blockR);
                Serial.print(" R+1: ");
                Serial.print((blockR + 1) & 3);
                Serial.print("\n\n"); */
        if (if_coll(blockX[nextBlock], blockY[nextBlock], (blockR[nextBlock] + 1) & 3) == 0)
          blockR[nextBlock] = (blockR[nextBlock] + 1) & 3;
        break;

      case moveDown:
        drop = 1;
        break;

      case gameStop:
        c = 'O';
        game_over = 1;
        xQueueSend(statusQ, &c, portMAX_DELAY);
        break;

      case gamePlay:
        reset_game();
        c = 'P';
        game_over = 0;
        xQueueSend(statusQ, &c, portMAX_DELAY);
        break;

      case paintPixel:
        leds[XYsafe(command.x, command.y)] = CRGB(command.r, command.g, command.b);
        repaint = 1;
        break;

      case screenFill:
        fill_solid(leds, NUM_LEDS, CRGB(command.r, command.g, command.b));
        repaint = 1;
        break;

      case screenBrightness:
        brightness = scale8(BRIGHTNESS, command.b);
        Serial.print("Bright=");
        Serial.println(brightness);
        FastLED.setBrightness(brightness);
        break;

      case screenImage:
        xQueueReceive(frameQ, leds, portMAX_DELAY);
        repaint = 1;
        break;

      default:
        break;
    }
  }
  if (repaint == 1)
  {
    FastLED.show();
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
      FastLED.setBrightness(scale8(brightness, (millis() * 256) / 5000));
    }
    else
    {
      FastLED.setBrightness(brightness);
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
          if (if_coll(blockX[nextBlock], blockY[nextBlock] + 1, blockR[nextBlock]) == 1)
            collide();
          else
            blockY[nextBlock]++;
        }
      }
      flash = 1 - flash;
      genDisplay();
      Refresh();
    }
    else
    {
      vTaskDelay(10);
    }
  }
}
