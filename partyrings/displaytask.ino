#include "common.h"
#include <FastLED.h>
#include "displayData.h"

TickType_t lastShow = xTaskGetTickCount();
uint8_t brightness = BRIGHTNESS;

void Refresh()
{
  const uint32_t delayTicks = 1000 / UPDATES_PER_SECOND / portTICK_PERIOD_MS;
  vTaskDelayUntil(&lastShow, delayTicks);
  FastLED.show();
  lastShow = xTaskGetTickCount();
}

void handle_controls()
{

  command_t command;

  while (uxQueueMessagesWaiting(commandQ) > 0)
  {
    char c;
    xQueueReceive(commandQ, &command, portMAX_DELAY);
    switch (command.action)
    {
    case screenBrightness:
      brightness = scale8(BRIGHTNESS, command.b);
      Serial.print("Bright=");
      Serial.println(brightness);
      FastLED.setBrightness(brightness);
      break;

    default:
      break;
    }
  }
}

CRGBPalette16 currentPalette;
TBlendType currentBlending;
uint8_t layout = 0;
uint8_t tightness = 3;

extern CRGBPalette16 myRedWhiteBluePalette;
extern const TProgmemPalette16 myRedWhiteBluePalette_p PROGMEM;
int8_t dir = 1;

void displayTask(void *params)
{

  Serial.print("Display task on core ");
  Serial.println(xPortGetCoreID());

  vTaskDelay(3000 / portTICK_PERIOD_MS); // power-up safety delay

  FastLED.addLeds<CHIPSET, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
  currentPalette = RainbowColors_p;
  currentBlending = LINEARBLEND;
  for (;;)
  {
    random16_add_entropy(esp_random() & 0xFFFF);
    for (uint8_t s = 0; s < 255; s++)
    {
      if (millis() < 5000)
      {
        FastLED.setBrightness(scale8(brightness, (millis() * 256) / 5000));
      }
      else
      {
        FastLED.setBrightness(brightness);
      }

      //      uint8_t sensorValue = analogRead(A0)>>2;
      //      Serial.println(sensorValue);
      //      if (sensorValue < 15) sensorValue=0;
      //      FastLED.setBrightness(  scale8(sensorValue,BRIGHTNESS) );
      handle_controls();

      genDisplay(s);
      Refresh();
    }
  }
}

void genDisplay(uint8_t startIndex)
{
  ChangePalettePeriodically();
  FillLEDsFromPaletteColors(startIndex);
}

void FillLEDsFromPaletteColors(uint8_t colorIndex)
{
  uint8_t brightness = 255;

  for (int i = 0; i < NUM_LEDS; i++)
  {
    uint8_t pos = i;
    switch (layout)
    {
    case 0: // matched clock
      pos = idx2Clock(i);
      break;
    case 1: // reversed clock
      pos = idx2Clock(i);
      if (idx2Eye(i))
        pos = 11 - pos;
      break;
    case 2: // Left Right
      pos = idx2X(i);
      break;
    case 3: // Up Down
      pos = idx2Y(i);
      break;
    case 4: // sequence
      pos = idx2Sequence(i);
      break;
    case 5: // Fig 8
      pos = idx2Fig8(i);
      break;
    case 6: // Solid
      pos = 0;
      break;
    }
    uint8_t colOffset = colorIndex + tightness * pos * dir;
    leds[i] = ColorFromPalette(currentPalette, colOffset, brightness, currentBlending);
  }
}

uint8_t idx2Eye(uint8_t i)
{
  return i >= 12;
}

uint8_t idx2Y(uint8_t i)
{
  if (i < 7)
    return i;
  if (i < 13)
    return 12 - i;
  if (i < 19)
    return i - 12;
  return 24 - i;
}

uint8_t idx2X(uint8_t i)
{
  if (i < 4)
    return i + 10;
  if (i < 10)
    return 16 - i;
  if (i < 12)
    return i - 2;
  if (i < 16)
    return i - 9;
  if (i < 22)
    return 21 - i;
  return i - 21;
}

uint8_t idx2Clock(uint8_t i)
{
  return i - 12 * idx2Eye(i);
}

uint8_t idx2Sequence(uint8_t i)
{
  if (i < 10)
    return i;
  if (i < 12)
    return i + 12;
  if (i < 16)
    return i + 6;
  return i - 6;
}

uint8_t idx2Fig8(uint8_t i)
{
  if (i < 10)
    return i;
  if (i < 12)
    return i + 12;
  if (i < 15)
    return 24 - i;
  return 36 - i;
}

// There are several different palettes of colors demonstrated here.
//
// FastLED provides several 'preset' palettes: RainbowColors_p, RainbowStripeColors_p,
// OceanColors_p, CloudColors_p, LavaColors_p, ForestColors_p, and PartyColors_p.
//
// Additionally, you can manually define your own color palettes, or you can write
// code that creates color palettes on the fly.  All are shown here.

// void newlayout() {
//   layout++;
//   if (layout > 6)
//     layout = 0;
// }
void ChangePalettePeriodically()
{
  uint8_t secondHand = (millis() / 4000) % 60;
  static uint8_t lastSecond = 99;

  if (lastSecond != secondHand)
  {
    lastSecond = secondHand;

    if (secondHand % 5 == 0)
    {
      layout = random8() % 7;
      dir = random8() % 2 * 2 - 1;
      tightness = random8() % 6 + 1;
      switch (random8() % 11)
      {
      case 0:
        currentPalette = RainbowColors_p;
        currentBlending = LINEARBLEND;
        break;
      case 1:
        currentPalette = RainbowStripeColors_p;
        currentBlending = NOBLEND;
        break;
      case 2:
        currentPalette = RainbowStripeColors_p;
        currentBlending = LINEARBLEND;
        break;
      case 3:
        SetupPurpleAndGreenPalette();
        currentBlending = LINEARBLEND;
        break;
      case 4:
        SetupTotallyRandomPalette();
        currentBlending = LINEARBLEND;
        break;
      case 5:
        SetupBlackAndWhiteStripedPalette();
        currentBlending = NOBLEND;
        break;
      case 6:
        SetupBlackAndWhiteStripedPalette();
        currentBlending = LINEARBLEND;
        break;
      case 7:
        currentPalette = CloudColors_p;
        currentBlending = LINEARBLEND;
        break;
      case 8:
        currentPalette = PartyColors_p;
        currentBlending = LINEARBLEND;
        break;
      case 9:
        currentPalette = myRedWhiteBluePalette_p;
        currentBlending = NOBLEND;
        break;
      case 10:
        currentPalette = myRedWhiteBluePalette_p;
        currentBlending = LINEARBLEND;
      }
    }
  }
}

// This function fills the palette with totally random colors.
void SetupTotallyRandomPalette()
{
  for (int i = 0; i < 16; i++)
  {
    currentPalette[i] = CHSV(random8(), 255, 255);
  }
}

// This function sets up a palette of black and white stripes,
// using code.  Since the palette is effectively an array of
// sixteen CRGB colors, the various fill_* functions can be used
// to set them up.
void SetupBlackAndWhiteStripedPalette()
{
  // 'black out' all 16 palette entries...
  fill_solid(currentPalette, 16, CRGB::Black);
  // and set every fourth one to white.
  currentPalette[0] = CRGB::White;
  currentPalette[4] = CRGB::White;
  currentPalette[8] = CRGB::White;
  currentPalette[12] = CRGB::White;
}

// This function sets up a palette of purple and green stripes.
void SetupPurpleAndGreenPalette()
{
  CRGB purple = CHSV(HUE_PURPLE, 255, 255);
  CRGB green = CHSV(HUE_GREEN, 255, 255);
  CRGB black = CRGB::Black;

  currentPalette = CRGBPalette16(
      green, green, black, black,
      purple, purple, black, black,
      green, green, black, black,
      purple, purple, black, black);
}

// This example shows how to set up a static color palette
// which is stored in PROGMEM (flash), which is almost always more
// plentiful than RAM.  A static PROGMEM palette like this
// takes up 64 bytes of flash.
const TProgmemPalette16 myRedWhiteBluePalette_p PROGMEM =
    {
        CRGB::Red,
        CRGB::Gray, // 'white' is too bright compared to red and blue
        CRGB::Blue,
        CRGB::Black,

        CRGB::Red,
        CRGB::Gray,
        CRGB::Blue,
        CRGB::Black,

        CRGB::Red,
        CRGB::Red,
        CRGB::Gray,
        CRGB::Gray,
        CRGB::Blue,
        CRGB::Blue,
        CRGB::Black,
        CRGB::Black};
