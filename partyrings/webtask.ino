#include "common.h"
#include "FS.h"
#include <LittleFS.h>
#include "secrets.h"
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>
#include "SpotifyArduino.h"
#include "SpotifyArduinoCert.h"
#include <string>

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

std::string oldId = "";
std::string currentId = "";

void printCurrentlyPlayingToSerial(CurrentlyPlaying currentlyPlaying);

void printCurrentlyPlayingToSerial(CurrentlyPlaying currentlyPlaying)
{
  // Use the details in this method or if you want to store them
  // make sure you copy them (using something like strncpy)
  // const char* artist =

  Serial.println("--------- Currently Playing ---------");

  Serial.print("Is Playing: ");
  if (currentlyPlaying.isPlaying)
  {
    Serial.println("Yes");
  }
  else
  {
    Serial.println("No");
  }

  Serial.print("Track: ");
  Serial.println(currentlyPlaying.trackName);
  Serial.print("Track URI: ");
  Serial.println(currentlyPlaying.trackUri);
  Serial.println();
  currentId = currentlyPlaying.id;

  Serial.println("Artists: ");
  for (int i = 0; i < currentlyPlaying.numArtists; i++)
  {
    Serial.print("Name: ");
    Serial.println(currentlyPlaying.artists[i].artistName);
    Serial.print("Artist URI: ");
    Serial.println(currentlyPlaying.artists[i].artistUri);
    Serial.println();
  }

  Serial.print("Album: ");
  Serial.println(currentlyPlaying.albumName);
  Serial.print("Album URI: ");
  Serial.println(currentlyPlaying.albumUri);
  Serial.println();

  long progress = currentlyPlaying.progressMs; // duration passed in the song
  long duration = currentlyPlaying.durationMs; // Length of Song
  Serial.print("Elapsed time of song (ms): ");
  Serial.print(progress);
  Serial.print(" of ");
  Serial.println(duration);
  Serial.println();

  float percentage = ((float)progress / (float)duration) * 100;
  int clampedPercentage = (int)percentage;
  Serial.print("<");
  for (int j = 0; j < 50; j++)
  {
    if (clampedPercentage >= (j * 2))
    {
      Serial.print("=");
    }
    else
    {
      Serial.print("-");
    }
  }
  Serial.println(">");
  Serial.println();

  // will be in order of widest to narrowest
  // currentlyPlaying.numImages is the number of images that
  // are stored
  for (int i = 0; i < currentlyPlaying.numImages; i++)
  {
    Serial.println("------------------------");
    Serial.print("Album Image: ");
    Serial.println(currentlyPlaying.albumImages[i].url);
    Serial.print("Dimensions: ");
    Serial.print(currentlyPlaying.albumImages[i].width);
    Serial.print(" x ");
    Serial.print(currentlyPlaying.albumImages[i].height);
    Serial.println();
  }
  Serial.println("------------------------");
}

void printAnalysisToSerial(Analysis analysis);

void printAnalysisToSerial(Analysis analysis)
{
  // Use the details in this method or if you want to store them
  // make sure you copy them (using something like strncpy)
  // const char* artist =

  Serial.println("--------- Analysis ---------");

  Serial.print("Tempo: ");
  Serial.println(analysis.tempo);
  Serial.print("Time signature: ");
  Serial.println(analysis.signature);
  Serial.println();

  Serial.println("NUmber of: ");
  Serial.print("Bars: ");
  Serial.println(analysis.numBars);
  Serial.print("Beats: ");
  Serial.println(analysis.numBeats);
  Serial.print("Tatums: ");
  Serial.println(analysis.numTatums);
  Serial.println();

  Serial.println("------------------------");
}

WiFiMulti wifiMulti;
WiFiClientSecure client;
SpotifyArduino spotify(client, spotifyId, spotifySecret, SPOTIFYTOKEN);

unsigned long delayBetweenRequests = 20000; // Time between requests (20 sec)
unsigned long requestDueTime;               // time when request due

void webTask(void *params)
{
  uint8_t data;
  char message[2];

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

  client.setCACert(spotify_server_cert);
  Serial.println(spotifySecret);
  Serial.println("Refreshing Access Tokens");
  if (!spotify.refreshAccessToken())
  {
    Serial.println("Failed to get access tokens");
  }

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
      Serial.print("Free Heap: ");
      Serial.println(ESP.getFreeHeap());

      Serial.println("getting currently playing song:");
      // Market can be excluded if you want e.g. spotify.getCurrentlyPlaying()
      int status = spotify.getCurrentlyPlaying(printCurrentlyPlayingToSerial, "");
      if (status == 200)
      {
        Serial.println("Successfully got currently playing");
        if (currentId != oldId)
        {
          oldId = currentId;
          int status = spotify.getTrackAnalysis(printAnalysisToSerial, currentId.c_str());
          if (status != 200)
          {
            Serial.print("Get analysis error: ");
            Serial.println(status);
          }
        }
      }
      else if (status == 204)
      {
        Serial.println("Doesn't seem to be anything playing");
      }
      else
      {
        Serial.print("Error: ");
        Serial.println(status);
      }
      requestDueTime = millis() + delayBetweenRequests;
    }
  }
}
