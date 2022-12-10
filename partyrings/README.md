Use the ESP32 sketch data uploader to load the index.html to LittleFS on the ESP32. See https://github.com/lorol/arduino-esp32fs-plugin

Copy secrets.h.template to secrets.h and add your wifi SSID and passwords

Update settings.h with your local URL if you are using the lightserver to get Spotify beat info. See https://github.com/cioportfolio/lightserver

Update settings.h as required e.g. led pins, matrix dimensions

When running the ESP32 will attempt to connect to wifi and serve a simple control page (brightness control and plain white/disco switch)
