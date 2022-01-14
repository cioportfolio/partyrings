#include "common.h"
#include "FS.h"
#include <LittleFS.h>

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

void handleTest(AsyncWebServerRequest *request)
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
}

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

void handleControl(AsyncWebServerRequest *request, const action_t a)
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
}

void webTask(void *params)
{
  Serial.print("Web task on core ");
  Serial.println(xPortGetCoreID());

  // Set web server port number to 80
  AsyncWebServer server(80);

  // Connect to Wi-Fi network with SSID and password
  Serial.print("Setting AP (Access Point)…");
  // Remove the password parameter, if you want the AP (Access Point) to be open
  WiFi.softAP(ssid);

  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(IP);

  // Activate mDNS this is used to be able to connect to the server
  // with local DNS hostmane esp8266.local
  if (MDNS.begin("leds")) {
    Serial.println("MDNS responder started");
  }
  
  if (!LittleFS.begin())
  {
    Serial.println("LittleFS Mount Failed");
    return;
  }

  server.on("/", handleRoot);
  server.on("/test", handleTest);
  server.on("/tetris", handleTetris);
  server.on("/pixel", handlePixel);
  server.on("/screen", handleScreen);
  server.onNotFound(handleNotFound);

  // Alow cross-origin requests
  DefaultHeaders::Instance().addHeader("Access-Control-Allow-Origin", "*");
  server.begin();

  for (;;)
  {
    vTaskDelay(1000);
  }
}
