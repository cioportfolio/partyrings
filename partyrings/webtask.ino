#include "common.h"
#include "FS.h"
#include <LittleFS.h>
#include <HTTPClient.h>
#include "secrets.h"

void handleNotFound(AsyncWebServerRequest *request)
{
  if (request->method() == HTTP_OPTIONS)
  {
    request->send(200);
  }
  else
  {
    String message = "File Not Found\n\n";
    message += "URI: ";
    message += request->url();
    message += "\nMethod: ";
    message += (request->method() == HTTP_GET) ? "GET" : "POST";
    message += "\nArguments: ";
    message += request->args();
    message += "\n";

    for (uint8_t i = 0; i < request->args(); i++)
    {
      message += " " + request->argName(i) + ": " + request->arg(i) + "\n";
    }

    request->send(404, "text/plain", message);
  }
}

/* void handleTest(AsyncWebServerRequest *request)
  {
  if (request->method() == HTTP_OPTIONS)
  {
    request->send(200);
  }
  else
  {
    String message = "Test OK\n\n";
    message += "URI: ";
    message += request->url();
    message += "\nMethod: ";
    message += (request->method() == HTTP_GET) ? "GET" : "POST";
    message += "\nArguments: ";
    message += request->args();
    message += "\n";

    for (uint8_t i = 0; i < request->args(); i++)
    {
      message += " " + request->argName(i) + ": " + request->arg(i) + "\n";
    }

    request->send(200, "text/plain", message);
  }
  } */

bool loadFromLittleFS(AsyncWebServerRequest *request, String path)
{
  String dataType = "text/html";

  Serial.print("Requested page -> ");
  Serial.println(path);
  if (LittleFS.exists(path))
  {
    File dataFile = LittleFS.open(path, "r");
    if (!dataFile)
    {
      handleNotFound(request);
      return false;
    }

    AsyncWebServerResponse *response = request->beginResponse(LittleFS, path, dataType);
    Serial.print("Real file path: ");
    Serial.println(path);

    request->send(response);

    dataFile.close();
  }
  else
  {
    handleNotFound(request);
    return false;
  }
  return true;
}

void handleRoot(AsyncWebServerRequest *request)
{
  if (request->method() == HTTP_OPTIONS)
  {
    request->send(200);
  }
  else
  {
    loadFromLittleFS(request, "/index.html");
  }
}

/* void handleControl(AsyncWebServerRequest *request, const action_t a)
  {
  command_t c = {a};
  request->send(200, "text/plain", "OK\n");
  xQueueSend(commandQ, &c, portMAX_DELAY);
  }

  void handleTetris(AsyncWebServerRequest *request)
  {
  if (request->method() == HTTP_OPTIONS)
  {
    request->send(200);
  }
  else
  {
    if (request->hasArg("move"))
    {
      char direction = request->arg("move").c_str()[0];
      switch (direction)
      {
      case 'l':
        handleControl(request, moveLeft);
        break;
      case 'r':
        handleControl(request, moveRight);
        break;
      case 'd':
        handleControl(request, moveDown);
        break;
      default:
        handleNotFound(request);
      }
    }
    else if (request->hasArg("rotate"))
    {
      char direction = request->arg("rotate").c_str()[0];
      switch (direction)
      {
      case 'l':
        handleControl(request, rotateLeft);
        break;
      case 'r':
        handleControl(request, rotateRight);
        break;
      default:
        handleNotFound(request);
      }
    }
    else if (request->hasArg("game"))
    {
      char command = request->arg("game").c_str()[0];
      switch (command)
      {
      case 'p':
        handleControl(request, gamePlay);
        break;
      case 'o':
        handleControl(request, gameStop);
        break;
      default:
        handleNotFound(request);
      }
    }
    else if (request->arg("query"))
    {
      char query = request->arg("query").c_str()[0];
      switch (query)
      {
      case 'p':
        if (game_over == 1)
        {
          request->send(200, "text/plain", "over");
        }
        else
        {
          request->send(200, "text/plain", "play");
        }
        break;
      case 's':
        request->send(200, "text/plain", String(score));
        break;
      default:
        handleNotFound(request);
      }
    }
    else
    {
      handleNotFound(request);
    }
  }
  }

  void handlePixel(AsyncWebServerRequest *request)
  {
  if (request->method() == HTTP_OPTIONS)
  {
    request->send(200);
  }
  else
  {
    if (game_over == 1)
    {
      uint8_t x = request->arg("x").toInt();
      uint8_t y = request->arg("y").toInt();
      uint8_t r = request->arg("r").toInt();
      uint8_t g = request->arg("g").toInt();
      uint8_t b = request->arg("b").toInt();
      command_t c = {paintPixel, x, y, r, g, b};
      request->send(200, "text/plain", "OK\n");
      xQueueSend(commandQ, &c, portMAX_DELAY);
    }
    else
    {
      request->send(409, "text/plain", "Cannot paint pixels while game is in progress\n");
    }
  }
  }

  uint8_t imageBuf[NUM_LEDS*3];

  void handleScreen(AsyncWebServerRequest *request)
  {
  if (request->method() == HTTP_OPTIONS)
  {
    request->send(200);
  }
  else
  {
    if (request->hasArg("brightness"))
    {
      uint8_t b = request->arg("brightness").toInt();
      command_t c = {screenBrightness, 0, 0, 0, 0, b};
      request->send(200, "text/plain", "OK\n");
      xQueueSend(commandQ, &c, portMAX_DELAY);
    }
    else if (game_over == 1)
    {
      if (request->hasArg("image"))
      {
        String i = request->arg("image");
        int sPtr = 0;
        int aPtr = 0;
        command_t c = {screenImage, 0, 0, 0, 0, 0};

        request->send(200, "text/plain", "OK\n");
        while (sPtr < i.length() && aPtr < NUM_LEDS*3)
        {
          imageBuf[aPtr] = strtoul(i.substring(sPtr, sPtr+2).c_str(),NULL,16);
          sPtr+=2;
          aPtr++;
        }
        for (; aPtr < NUM_LEDS*3; aPtr++)
        {
          imageBuf[aPtr]=0;
        }
        xQueueSend(commandQ, &c, portMAX_DELAY);
        xQueueSend(frameQ, imageBuf, portMAX_DELAY);
      }
      else
      {
        uint8_t r = request->arg("r").toInt();
        uint8_t g = request->arg("g").toInt();
        uint8_t b = request->arg("b").toInt();
        command_t c = {screenFill, 0, 0, r, g, b};
        request->send(200, "text/plain", "OK\n");
        xQueueSend(commandQ, &c, portMAX_DELAY);
      }
    }
    else
    {
      request->send(409, "text/plain", "Cannot fill screen while game is in progress\n");
    }
  }
  } */

void wsHandler(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len)
{
  command_t c;
  switch (type)
  {
  case WS_EVT_CONNECT:
    Serial.println("Websocket client connection received");
    break;
  case WS_EVT_DISCONNECT:
    Serial.println("Client disconnected");
    break;
  case WS_EVT_DATA:
    Serial.print("Socket data: ");
    Serial.println((char)data[0]);
    switch ((char)data[0])
    {
    case 'B':
      c = {screenBrightness, data[1]};
      xQueueSend(commandQ, &c, portMAX_DELAY);
      break;
    case 'D':
      c = {modeDisco, 0};
      xQueueSend(commandQ, &c, portMAX_DELAY);
      break;
    case 'W':
      c = {modeWhite, 0};
      xQueueSend(commandQ, &c, portMAX_DELAY);
      break;
      /* case 'L':
        c = {moveLeft};
        xQueueSend(commandQ, &c, portMAX_DELAY);
        break;
        case 'R':
        c = {moveRight};
        xQueueSend(commandQ, &c, portMAX_DELAY);
        break;
        case 'D':
        c = {moveDown};
        xQueueSend(commandQ, &c, portMAX_DELAY);
        break;
        case 'l':
        c = {rotateLeft};
        xQueueSend(commandQ, &c, portMAX_DELAY);
        break;
        case 'r':
        c = {rotateRight};
        xQueueSend(commandQ, &c, portMAX_DELAY);
        break;
        case 'P':
        c = {gamePlay};
        xQueueSend(commandQ, &c, portMAX_DELAY);
        break;
        case 'O':
        c = {gameStop};
        xQueueSend(commandQ, &c, portMAX_DELAY);
        break; */
    }
    break;
  }
}

WiFiMulti wifiMulti;
HTTPClient http;

unsigned long delayBetweenRequests = 1000; // Time between requests (20 sec)
unsigned long requestDueTime;              // time when request due
uint8_t currentTrack = 255;
uint8_t newTrack = 255;
uint32_t estStart = millis() / 10;
uint32_t newEst = estStart;

void webTask(void *params)
{
  uint8_t data;
  command_t cmd;

  Serial.print("Web task on core ");
  Serial.println(xPortGetCoreID());

  // Set web server port number to 80
  AsyncWebServer server(80);
  AsyncWebSocket ws("/ws");

  // Connect to Wi-Fi network with SSID and password
  // Serial.print("Setting AP (Access Point)…");
  // Remove the password parameter, if you want the AP (Access Point) to be open
  // WiFi.softAP(ssid);

  // IPAddress IP = WiFi.softAPIP();

  wifiMulti.addAP(SECRETSSID1, SECRETPASSWORD1);
  wifiMulti.addAP(SECRETSSID2, SECRETPASSWORD2);
  if (wifiMulti.run() == WL_CONNECTED)
  {
    IPAddress IP = WiFi.localIP();
    String out = "Partyrings: ";
    out += IP.toString();
    Serial.println(out);
  }

  http.setReuse(true);

  // Activate mDNS this is used to be able to connect to the server
  // with local DNS hostmane esp8266.local
  // initialize mDNS service
  esp_err_t err = mdns_init();
  if (err)
  {
    printf("MDNS Init failed: %d\n", err);
    return;
  }

  // set hostname
  mdns_hostname_set("my-esp32");
  // set default instance
  mdns_instance_name_set("partyrings");

  if (!LittleFS.begin())
  {
    Serial.println("LittleFS Mount Failed");
    return;
  }

  server.on("/", handleRoot);
  /* server.on("/test", handleTest);
    server.on("/tetris", handleTetris);
    server.on("/pixel", handlePixel);
    server.on("/screen", handleScreen); */
  server.onNotFound(handleNotFound);
  ws.onEvent(wsHandler);
  server.addHandler(&ws);

  // Alow cross-origin requests
  DefaultHeaders::Instance().addHeader("Access-Control-Allow-Origin", "*");
  server.begin();

  for (;;)
  {
    vTaskDelay(10);
    /*while (uxQueueMessagesWaiting(scoreQ)>0)
      {
      xQueueReceive(scoreQ, &data, portMAX_DELAY);
      message[0] = 'S';
      message[1] = (char)data;
      ws.textAll(message, 2);
      }
      while (uxQueueMessagesWaiting(statusQ)>0)
      {
      xQueueReceive(statusQ, &data, portMAX_DELAY);
      message[0] = (char)data;
      ws.textAll(message, 2);
      } */
    if (millis() > requestDueTime)
    {
      // Serial.print("Free Heap: ");
      // Serial.println(ESP.getFreeHeap());

      // Serial.println("getting currently playing song and progress:");

      http.begin(String(URL) + "/progress");
      unsigned long apiTime = millis();
      int status = http.GET();
      if (status > 0)
      {
        if (status == HTTP_CODE_OK)
        {
          WiFiClient *stream = http.getStreamPtr();
          if (stream->read())
          {
            // Serial.println("Successfully got currently playing");
            newTrack = stream->read();
            // Serial.print("Track: ");
            // Serial.println(newTrack);
            uint16_t progress = stream->read();
            progress = (progress << 8) | stream->read();
            newEst = (apiTime / 10 - progress);
          }
          else
          {
            Serial.println("No progress data");
          }
        }
        else
        {
          Serial.print("Error: ");
          Serial.println(status);
        }
      }
      http.end();
      requestDueTime = millis() + delayBetweenRequests;
      if (currentTrack != newTrack)
      {
        // Serial.println("New track, need new analysis");
        http.begin(String(URL) + "/analysis");
        int status = http.GET();
        if (status > 0)
        {
          if (status == HTTP_CODE_OK)
          {
            WiFiClient *stream = http.getStreamPtr();
            if (stream->read())
            {
              Serial.println("Successfully got analysis");
              analIn.tempo = stream->read();
              Serial.print("Tempo: ");
              Serial.println(analIn.tempo);
              uint8_t temp = stream->read();
              analIn.barCount = (temp << 8) | stream->read();
              Serial.print("Bar Count: ");
              Serial.println(analIn.barCount);
              temp = stream->read();
              analIn.beatCount = (temp << 8) | stream->read();
              Serial.print("Beat Count: ");
              Serial.println(analIn.beatCount);
              temp = stream->read();
              analIn.tatumCount = (temp << 8) | stream->read();
              Serial.print("Tatum Count: ");
              Serial.println(analIn.tatumCount);

              if (analIn.beatCount > MAX_BEATS)
                analIn.beatCount = MAX_BEATS;
              if (analIn.tatumCount > MAX_TATUMS)
                analIn.tatumCount = MAX_TATUMS;

              for (int i = 0; i < analIn.barCount && i < MAX_BARS; i++)
              {
                temp = stream->read();
                analIn.bars[i] = (temp << 8) | stream->read();
              }
              if (analIn.barCount > MAX_BARS)
              {
                Serial.print("Skipping Bars: ");
                Serial.println(analIn.barCount - MAX_BARS);
                for (int i = 0; i < (analIn.barCount - MAX_BARS) * 2; i++)
                {
                  temp = stream->read();
                }
                analIn.barCount = MAX_BARS;
              }
              for (int i = 0; i < analIn.beatCount && i < MAX_BEATS; i++)
              {
                temp = stream->read();
                analIn.beats[i] = (temp << 8) | stream->read();
              }
              if (analIn.beatCount > MAX_BEATS)
              {
                Serial.print("Skipping Beats: ");
                Serial.println(analIn.beatCount - MAX_BEATS);
                for (int i = 0; i < (analIn.beatCount - MAX_BEATS) * 2; i++)
                {
                  temp = stream->read();
                }
                analIn.beatCount = MAX_BEATS;
              }
              for (int i = 0; i < analIn.tatumCount && i < MAX_TATUMS; i++)
              {
                temp = stream->read();
                analIn.tatums[i] = (temp << 8) | stream->read();
              }
              if (analIn.tatumCount > MAX_TATUMS)
              {
                Serial.print("Skipping tatums: ");
                Serial.println(analIn.tatumCount - MAX_TATUMS);
                for (int i = 0; i < (analIn.tatumCount - MAX_TATUMS) * 2; i++)
                {
                  temp = stream->read();
                }
                analIn.tatumCount = MAX_TATUMS;
              }
              xQueueSend(analysisQ, &analIn, portMAX_DELAY);
              estStart = newEst;
              cmd = {newAnalysis, 0, estStart};
              xQueueSend(commandQ, &cmd, portMAX_DELAY);
              currentTrack = newTrack;
            }
            else
            {
              Serial.println("No analysis data");
            }
          }
          else
          {
            Serial.print("Error: ");
            Serial.println(status);
          }
        }
        http.end();
      }
      else
      {
        if ((newEst > estStart && newEst - estStart > 50) || (estStart > newEst && estStart - newEst > 50))
        {
          estStart = (newEst + estStart) / 2;
          cmd = {newStart, 0, estStart};
          xQueueSend(commandQ, &cmd, portMAX_DELAY);
        }
      }
    }
  }
}
