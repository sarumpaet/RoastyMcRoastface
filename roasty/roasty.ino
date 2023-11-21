/*
 Roasty McRoastface
 The overly simple coffee roasting controller
 https://github.com/sarumpaet/RoastyMcRoastface

 based on ESP32 HTTP IoT Server Example for Wokwi.com
 https://wokwi.com/arduino/projects/320964045035274834
*/

#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <uri/UriBraces.h>
#include <ESP32Servo.h>
#include <ESP32Time.h>
#include <WiFiManager.h>

Servo myservo;
ESP32Time rtc(0);

WebServer server(80);

const int LED1 = 2;

int curstate = 0;
int curadc = 0;
int curservo = 0;
unsigned long curtime = 0;
unsigned long lastping = 0;

void sendHtml() {
  String response = R"(
    nothing here yet
  )";
  server.send(200, "text/html", response);
}

void setup(void) {
  Serial.begin(115200);
  pinMode(LED1, OUTPUT);
  WiFiManager wm;
  bool res;
  //res = wm.preloadWiFi("Wokwi-GUEST", "");
  res = wm.autoConnect("Roasty McRoastface", "roastyroast");
  if(!res) {
    Serial.println("Failed to connect");
    // ESP.restart();
  } else {
    Serial.println("connected...");
  }

  //WiFi.begin(WIFI_SSID, WIFI_PASSWORD, WIFI_CHANNEL);
  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(100);
    Serial.print(".");
  }
  Serial.println(" Connected!");

  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  server.on("/", sendHtml);

  server.on(UriBraces("/state/{}"), []() {
    String newstate = server.pathArg(0);
    curstate = newstate.toInt();
    server.send(200, "text/html", "ok");
  });

  server.on(UriBraces("/servo/{}"), []() {
    String newservo = server.pathArg(0);
    lastping = curtime;
    curservo = newservo.toInt();
    myservo.write(curservo);
    Serial.print("Time: ");
    Serial.print(curtime);
    Serial.print(",  adc: ");
    Serial.print(curadc);
    Serial.print(", servo: ");
    Serial.println(curservo);
    server.sendHeader("Access-Control-Allow-Origin", "*");
    server.sendHeader("Access-Control-Allow-Methods", "*");
    server.send(200, "text/html", "ok");
  });

  server.on(UriBraces("/status"), []() {
    String response = "{\"adc\": CURADC, \"servo\": CURSERVO, \"time\": CURTIME, \"state\": CURSTATE}";
    response.replace("CURADC", String(curadc));
    response.replace("CURSERVO", String(curservo));
    response.replace("CURTIME", String(curtime));
    response.replace("CURSTATE", String(curstate));
    lastping = curtime;
    server.sendHeader("Access-Control-Allow-Origin", "*");
    server.sendHeader("Access-Control-Allow-Methods", "*");
    server.send(200, "application/json", response);
  });

  server.begin();
  Serial.println("HTTP server started");

  myservo.attach(13);

  rtc.setTime(0, 0, 0, 1, 1, 1970);
}

void loop(void) {
  unsigned long adc = 0;
  for ( int i = 0; i < 10; i++ ) adc += analogRead(35);
  curadc = adc / 10;

  curtime = rtc.getEpoch() * 1000 + rtc.getMillis();
  if((lastping == 0) || ((lastping + 15000) < curtime)) {
    // web client timeout
    myservo.write(0);
    digitalWrite(LED1, (curtime&255)>100);
  } else {
    digitalWrite(LED1, (curtime&4095)>1000);
  }

  server.handleClient();

  delay(2);
}


