#include "common.h"
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include "FS.h"
#include <LittleFS.h>


void handleNotFound(AsyncWebServerRequest *request) {
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += request->url();
  message += "\nMethod: ";
  message += (request->method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += request->args();
  message += "\n";
 
  for (uint8_t i = 0; i < request->args(); i++) {
    message += " " + request->argName(i) + ": " + request->arg(i) + "\n";
  }
 
  request->send(404, "text/plain", message);
}

void handleTest(AsyncWebServerRequest *request) {
  String message = "Test OK\n\n";
  message += "URI: ";
  message += request->url();
  message += "\nMethod: ";
  message += (request->method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += request->args();
  message += "\n";
 
  for (uint8_t i = 0; i < request->args(); i++) {
    message += " " + request->argName(i) + ": " + request->arg(i) + "\n";
  }
 
  request->send(200, "text/plain", message);
}

bool loadFromLittleFS(AsyncWebServerRequest *request, String path) {
  String dataType = "text/html";
 
  Serial.print("Requested page -> ");
  Serial.println(path);
  if (LittleFS.exists(path)){
      File dataFile = LittleFS.open(path, "r");
      if (!dataFile) {
          handleNotFound(request);
          return false;
      }
 
        AsyncWebServerResponse *response = request->beginResponse(LittleFS, path, dataType);
        Serial.print("Real file path: ");
        Serial.println(path);
 
        request->send(response);
 
 
      dataFile.close();
  }else{
      handleNotFound(request);
      return false;
  }
  return true;
}
 
 
void handleRoot(AsyncWebServerRequest *request) {
    loadFromLittleFS(request, "/index.html");
}

void handleControl(AsyncWebServerRequest *request, const char c)
{
                  xQueueSend(commandQ, &c, portMAX_DELAY);
                request->send(200,"OK");
}

void handleTetris(AsyncWebServerRequest *request)
{ 
  if (request->hasArg("move"))
  {
    char direction = request->arg("move").c_str()[0];
    switch (direction)
    {
      case 'l':
        handleControl(request, 'L');
        break;
      case 'r':
        handleControl(request, 'R');
        break;
      case 'd':
        handleControl(request, 'D');
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
        handleControl(request, 'l');
        break;
      case 'r':
        handleControl(request, 'r');
        break;
      default:
        handleNotFound(request);
    }
  }
  else  if (request->hasArg("game"))
  {
    char command = request->arg("game").c_str()[0];
    switch (command)
    {
      case 'p':
        handleControl(request, 'P');
        break;
      case 'o':
        handleControl(request, 'X');
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

void webTask(void *params)
{
  char command = ' ';

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

  if (!LittleFS.begin()) {
    Serial.println("LittleFS Mount Failed");
    return;
  }

  server.on("/", handleRoot);
  server.on("/test", handleTest);
  server.on("/tetris", handleTetris);
  server.onNotFound(handleNotFound);
  server.begin();

  for (;;)
  {
         vTaskDelay(1000);
  }
}
