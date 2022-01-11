#include "common.h"
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include "FS.h"
#include <LITTLEFS.h>

void sendAPI(WiFiClient c, char* msg)
{
  c.println(msg);
  c.println();
}

void sendOK(WiFiClient c)
{
  sendAPI(c, "OK");
}

void webTask(void *params)
{
  char command = ' ';

  Serial.print("Web task on core ");
  Serial.println(xPortGetCoreID());

  // Set web server port number to 80
  WiFiServer server(80);

  // Variable to store the HTTP request
  String header;

  // Connect to Wi-Fi network with SSID and password
  Serial.print("Setting AP (Access Point)…");
  // Remove the password parameter, if you want the AP (Access Point) to be open
  WiFi.softAP(ssid);

  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(IP);

  if (!LITTLEFS.begin()) {
    Serial.println("LITTLEFS Mount Failed");
    return;
  }

  server.begin();

  for (;;)
  {
    WiFiClient client = server.available(); // Listen for incoming clients

    if (client)
    { // If a new client connects,
      Serial.println("New Client."); // print a message out in the serial port
      String currentLine = "";       // make a String to hold incoming data from the client
      while (client.connected())
      { // loop while the client's connected
        if (client.available())
        { // if there's bytes to read from the client,
          char c = client.read(); // read a byte, then
          Serial.write(c);        // print it out the serial monitor
          header += c;
          if (c == '\n')
          { // if the byte is a newline character
            // if the current line is blank, you got two newline characters in a row.
            // that's the end of the client HTTP request, so send a response:
            if (currentLine.length() == 0)
            {
              // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
              // and a content-type so the client knows what's coming, then a blank line:
              client.println("HTTP/1.1 200 OK");
              client.println("Content-type:text/html");
              client.println("Connection: close");
              client.println();


              if (header.indexOf("GET /left") >= 0)
              {
                Serial.println("Move left button");
                command = 'L';
                xQueueSend(commandQ, &command, portMAX_DELAY);
                sendOK(client);
              }
              else if (header.indexOf("GET /right") >= 0)
              {
                Serial.println("Move right button");
                command = 'R';
                xQueueSend(commandQ, &command, portMAX_DELAY);
                sendOK(client);

              }
              else if (header.indexOf("GET /rotateleft") >= 0)
              {
                Serial.println("Rotate left button");
                command = 'l';
                xQueueSend(commandQ, &command, portMAX_DELAY);
                sendOK(client);

              }
              else if (header.indexOf("GET /rotateright") >= 0)
              {
                Serial.println("Rotate right button");
                command = 'r';
                xQueueSend(commandQ, &command, portMAX_DELAY);
                sendOK(client);

              }
              else if (header.indexOf("GET /giveup") >= 0)
              {
                Serial.println("Exit button");
                command = 'X';
                xQueueSend(commandQ, &command, portMAX_DELAY);
                sendOK(client);

              }
              else if (header.indexOf("GET /play") >= 0)
              {
                Serial.println("Play button");
                command = 'P';
                xQueueSend(commandQ, &command, portMAX_DELAY);
                sendOK(client);

              }
              else if (header.indexOf("GET /drop") >= 0)
              {
                Serial.println("Down button");
                command = 'D';
                xQueueSend(commandQ, &command, portMAX_DELAY);
                sendOK(client);

              }
              else if (header.indexOf("GET /status") >= 0)
              {
                Serial.println("Status");
                if (game_over == 1)
                {
                  sendAPI(client, "over");
                }
                else
                {
                  sendAPI(client, "play");
                }
              }
              else if (header.indexOf("GET /score") >= 0)
              {
                Serial.println("Score");
                client.print(score);
                client.println();
                client.println();
              }
              else
              {

                /*// Display the HTML web page
                  client.println("<!DOCTYPE html><html>");
                  client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
                  client.println("<link rel=\"icon\" href=\"data:,\">");
                  // CSS to style the on/off buttons
                  // Feel free to change the background-color and font-size attributes to fit your preferences
                  client.println("<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}");
                  client.println(".button { background-color: #4CAF50; border: none; color: white; padding: 16px 40px;");
                  client.println("text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}");
                  client.println(".button2 {background-color: #555555;}</style></head>");

                  // Web Page Heading
                  client.println("<body><h1>TechJam Tetris</h1>");
                  client.print("<h2>Score : ");
                  client.print(score);
                  client.println("</h2>");

                  if (game_over == 0)
                  {
                  client.println("<p>");
                  client.println("<a href=\"/giveup\"><button class=\"button\">Stop &#9632</button></a>");
                  client.println("</p>");
                  client.println("<p>");
                  client.println("<a href=\"/left\"><button class=\"button\">&#8678</button></a>");
                  client.println("<a href=\"/right\"><button class=\"button\">&#8680</button></a>");
                  client.println("</p>");
                  client.println("<p>");
                  client.println("<a href=\"/drop\"><button class=\"button\">&#8681</button></a>");
                  client.println("</p>");
                  client.println("<p>");
                  client.println("<a href=\"/rotateleft\"><button class=\"button\">&#8630</button></a>");
                  client.println("<a href=\"/rotateright\"><button class=\"button\">&#8631</button></a>");
                  client.println("</p>");
                  }
                  else
                  {
                  client.println("<p>");
                  client.println("<h2>GAME OVER</h2>");
                  client.println("</p>");
                  client.println("<p>");
                  client.println("<a href=\"/play\"><button class=\"button\">Play &#9654</button></a>");
                  client.println("</p>");
                  }
                  client.println("<script>");
                  client.println("function api(msg){</body></html>");

                  // The HTTP response ends with another blank line
                  client.println();
                  // Break out of the while loop */

                String dataType = "text/html";
                File dataFile = LITTLEFS.open("/index.html");
                if (!dataFile)
                {
                  client.println("Index.html not found");
                  client.println("");
                }
                else
                {
                  
                  server.streamFile(dataFile, dataType);
                }
                dataFile.close();
                break;
              }
            }
            else
            { // if you got a newline, then clear currentLine
              currentLine = "";
            }
          }
          else if (c != '\r')
          { // if you got anything else but a carriage return character,
            currentLine += c; // add it to the end of the currentLine
          }
        } else {
          vTaskDelay(10);
        }
      }
      // Clear the header variable
      header = "";
      // Close the connection
      client.stop();
      Serial.println("Client disconnected.");
      Serial.println("");
    } else {
      vTaskDelay(10);
    }
  }
}
