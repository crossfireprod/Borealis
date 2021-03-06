/* === INCLUDES === */
  // OTA Update
  #include <ESP8266WiFi.h>
  #include <ESP8266mDNS.h>
  #include <WiFiUdp.h>
  #include <ArduinoOTA.h>

  // WS2812
  #include <WS2812FX.h>

  // aREST
  #include <aREST.h>
  #include <PubSubClient.h> // cloud

/* === DEFINITIONS === */
  // OTA Update
  const char* ssid      = "";
  const char* password  = "";
  #define     HOSTNAME  "ESP_"

  // WS2812
  #define LED_COUNT   108
  #define LED_PIN     2

  WS2812FX ws2812fx = WS2812FX(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ400);

  // aREST
  aREST rest = aREST();

  #define LISTEN_PORT           80
  WiFiServer server(LISTEN_PORT);

  // Function Prototypes
  int start(String);
  int stop(String);
  int setMode(String);
  int setSpeed(String);
  int setColor(String);
  int setBrightness(String);

  int isRunning(String);
  int getModeCount(String);

  int restart(String);


void setup() {
  // UART - Debug Messages
  Serial.begin(115200);
  Serial.println("Booting");

  // WiFi
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("Connection Failed! Rebooting...");
    delay(5000);
    ESP.restart();
  }

  // OTA
  // Port defaults to 8266
  // ArduinoOTA.setPort(8266);

  // Hostname defaults to esp8266-[ChipID]
  //ArduinoOTA.setHostname(HOSTNAME);

  // No authentication by default
  //ArduinoOTA.setPassword("password");

  // Password can be set with it's md5 value as well
  // MD5(admin) = 21232f297a57a5a743894a0e4a801fc3
  // ArduinoOTA.setPasswordHash("21232f297a57a5a743894a0e4a801fc3");

  ArduinoOTA.onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH)
      type = "sketch";
    else // U_SPIFFS
      type = "filesystem";

    // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
    Serial.println("Start updating " + type);

    // Clean Exit?
    ws2812fx.stop();

  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR) Serial.println("End Failed");
  });
  ArduinoOTA.begin();
  Serial.println("Ready");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  // WS2812
  ws2812fx.init();
  ws2812fx.start();
  ws2812fx.setMode(FX_MODE_STATIC);

  // aREST
    // Functions to be exposed
    rest.function("start", start);
    rest.function("stop", stop);
    rest.function("setMode", setMode);
    rest.function("setSpeed", setSpeed);
    rest.function("setColor", setColor);
    rest.function("setBrightness", setBrightness);
    rest.function("isRunning", isRunning);
    rest.function("getModeCount", getModeCount);

    rest.function("restart", restart);

    // Give name & ID to the device (ID should be 6 characters long)
    rest.set_id("1");
    rest.set_name("esp8266");

    // Start the server
    server.begin();
    Serial.println("Server Started");

  }


void loop() {
  // OTA
  ArduinoOTA.handle();

  // WS2812FX
  ws2812fx.service();

  // aREST
  WiFiClient client = server.available();
  if (!client) {
    return;
  }
  while(!client.available()){
    delay(1);
  }
  rest.handle(client);

  // Connect to the cloud
  rest.handle(client);
}


int start(String) {
  ws2812fx.start();

  return 1;
}


int stop(String) {
  ws2812fx.stop();

  return 1;
}


int setMode(String command) {
  int mode = command.toInt();

  ws2812fx.setMode(mode);

  return mode;
}


int setSpeed(String command) {
  int speed = command.toInt();

  ws2812fx.setSpeed(speed);

  return speed;
}


int setColor(String command) {
  int color = command.toInt();

  ws2812fx.setColor(color);

  return 1;
}


int setBrightness(String command) {
  int brightness = command.toInt();

  ws2812fx.setBrightness(brightness);

  return brightness;
}


int isRunning(String) {
  return ws2812fx.isRunning();
}


int getModeCount(String) {
  return ws2812fx.getModeCount();
}

int restart(String) {
  ws2812fx.stop();
  ESP.restart();

  return 1;
}
