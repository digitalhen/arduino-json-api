/*
  Arduino JSON API & Web Server

  Control digital pins over WiFi via HTTP GET requests.
  Built for remote control of a door buzzer.

  Usage: http://<ip>/<ACTION>/<PIN>
  Actions: STATE, ON, OFF, BUZZ
  Example: http://192.168.1.10/BUZZ/2

  Circuit:
  * WiFi shield attached
  * Devices connected to digital pins

  created 26 Nov 2016
  by digitalhen (Henry Williams)
*/
#include <SPI.h>
#include <WiFi101.h>
#include <ArduinoJson.h>

char ssid[] = "ssid";
char pass[] = "password";

const int validPins[] = {1, 2, 3, 4, 6, 8, 9, 10, 11, 12, 13};
const int validPinCount = sizeof(validPins) / sizeof(validPins[0]);

const unsigned long BUZZ_DURATION_MS = 5000;

// Non-blocking buzz state
int buzzPin = -1;
unsigned long buzzStartTime = 0;

int wifiStatus = WL_IDLE_STATUS;
WiFiServer server(80);

bool isValidPin(int pin) {
  for (int i = 0; i < validPinCount; i++) {
    if (validPins[i] == pin) return true;
  }
  return false;
}

// Extract the nth segment from a '/'-delimited path (e.g. "/BUZZ/2")
// Segment 0 = "" (before first slash), 1 = "BUZZ", 2 = "2"
String getPathSegment(const String& path, int index) {
  int current = 0;
  String segment = "";

  for (unsigned int i = 0; i < path.length(); i++) {
    if (path[i] == '/') {
      if (current == index) return segment;
      current++;
      segment = "";
    } else {
      segment += path[i];
    }
  }
  return (current == index) ? segment : "";
}

// Extract the URL path from an HTTP request line like "GET /BUZZ/2 HTTP/1.1"
String extractPath(const String& requestLine) {
  int start = requestLine.indexOf(' ');
  if (start < 0) return "";
  start++; // skip the space
  int end = requestLine.indexOf(' ', start);
  if (end < 0) return requestLine.substring(start);
  return requestLine.substring(start, end);
}

void sendJsonResponse(WiFiClient& client, const String& statusCode,
                       const String& action, int pin, int val) {
  client.println("HTTP/1.1 " + statusCode);
  client.println("Content-Type: application/json");
  client.println("Connection: close");
  client.println();

  DynamicJsonBuffer jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();

  if (statusCode.startsWith("200")) {
    root["status"] = "ok";
  } else {
    root["status"] = "error";
  }

  if (action.length() > 0) {
    root["action"] = action;
  }

  if (pin > 0) {
    JsonArray& pins = root.createNestedArray("data");
    JsonObject& pinObj = pins.createNestedObject();
    pinObj["pin"] = pin;
    pinObj["value"] = val;
  }

  root.printTo(client);
  client.println();
}

void setup() {
  Serial.begin(9600);

  if (WiFi.status() == WL_NO_SHIELD) {
    Serial.println("WiFi shield not present");
    while (true);
  }

  while (wifiStatus != WL_CONNECTED) {
    Serial.print("Connecting to: ");
    Serial.println(ssid);
    wifiStatus = WiFi.begin(ssid, pass);
    delay(10000);
  }

  // Initialize all valid pins as OUTPUT
  for (int i = 0; i < validPinCount; i++) {
    pinMode(validPins[i], OUTPUT);
  }

  server.begin();
  printWifiStatus();
}

void loop() {
  // Handle non-blocking buzz timeout
  if (buzzPin >= 0 && (millis() - buzzStartTime >= BUZZ_DURATION_MS)) {
    digitalWrite(buzzPin, LOW);
    Serial.print("Buzz ended on pin ");
    Serial.println(buzzPin);
    buzzPin = -1;
  }

  WiFiClient client = server.available();
  if (!client) return;

  Serial.println("New client");
  String currentLine = "";
  String requestLine = "";

  while (client.connected()) {
    if (!client.available()) continue;

    char c = client.read();
    Serial.write(c);

    if (c == '\n') {
      if (currentLine.length() == 0) {
        // End of HTTP headers — process the request
        String path = extractPath(requestLine);
        String action = getPathSegment(path, 1);
        int actionPin = getPathSegment(path, 2).toInt();

        if (action == "STATE" && actionPin > 0 && isValidPin(actionPin)) {
          int val = digitalRead(actionPin);
          sendJsonResponse(client, "200 OK", action, actionPin, val);

        } else if (action == "STATE" && actionPin == 0) {
          // Return the state of all valid pins
          client.println("HTTP/1.1 200 OK");
          client.println("Content-Type: application/json");
          client.println("Connection: close");
          client.println();

          DynamicJsonBuffer jsonBuffer;
          JsonObject& root = jsonBuffer.createObject();
          root["status"] = "ok";
          root["action"] = "STATE";
          JsonArray& pins = root.createNestedArray("data");
          for (int i = 0; i < validPinCount; i++) {
            JsonObject& pinObj = pins.createNestedObject();
            pinObj["pin"] = validPins[i];
            pinObj["value"] = digitalRead(validPins[i]);
          }
          root.printTo(client);
          client.println();

        } else if (action == "ON" && actionPin > 0 && isValidPin(actionPin)) {
          digitalWrite(actionPin, HIGH);
          int val = digitalRead(actionPin);
          sendJsonResponse(client, "200 OK", action, actionPin, val);

        } else if (action == "OFF" && actionPin > 0 && isValidPin(actionPin)) {
          digitalWrite(actionPin, LOW);
          int val = digitalRead(actionPin);
          sendJsonResponse(client, "200 OK", action, actionPin, val);

        } else if (action == "BUZZ" && actionPin > 0 && isValidPin(actionPin)) {
          digitalWrite(actionPin, HIGH);
          buzzPin = actionPin;
          buzzStartTime = millis();
          int val = digitalRead(actionPin);
          sendJsonResponse(client, "200 OK", action, actionPin, val);

        } else {
          sendJsonResponse(client, "400 Bad Request", "", 0, 0);
        }

        break;
      } else {
        // Save the first line (request line)
        if (requestLine.length() == 0) {
          requestLine = currentLine;
        }
        currentLine = "";
      }
    } else if (c != '\r') {
      currentLine += c;
    }
  }

  client.stop();
  Serial.println("Client disconnected");
}

void printWifiStatus() {
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  long rssi = WiFi.RSSI();
  Serial.print("Signal strength (RSSI): ");
  Serial.print(rssi);
  Serial.println(" dBm");

  Serial.print("Open browser to: http://");
  Serial.println(ip);
}
