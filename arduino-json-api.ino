#include <SPI.h>
#include <WiFi101.h>
#include <ArduinoJson.h>

char ssid[] = "ssid";      //  your network SSID (name)
char pass[] = "password";   // your network password
int keyIndex = 0;                 // your network key Index number (needed only for WEP)
String action = ""; // what is the action
int actionPin = 0; // which pin are we performing the action on
int validPins[] = {1,2,3,4,6,8,9,10,11,12,13};
int validPinSize = 11;
String getRequest = "";

int status = WL_IDLE_STATUS;
WiFiServer server(80);

void setup() {
  Serial.begin(9600);      // initialize serial communication


  // check for the presence of the shield:
  if (WiFi.status() == WL_NO_SHIELD) {
    Serial.println("WiFi shield not present");
    while (true);       // don't continue
  }

  // attempt to connect to Wifi network:
  while ( status != WL_CONNECTED) {
    Serial.print("Attempting to connect to Network named: ");
    Serial.println(ssid);                   // print the network name (SSID);

    // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
    status = WiFi.begin(ssid, pass);
    // wait 10 seconds for connection:
    delay(10000);
  }
  server.begin();                           // start the web server on port 80
  printWifiStatus();                        // you're connected now, so print out the status
}


void loop() {
  WiFiClient client = server.available();   // listen for incoming clients

  if (client) {                             // if you get a client,
    Serial.println("new client");           // print a message out the serial port
    String currentLine = "";                // make a String to hold incoming data from the client
    while (client.connected()) {            // loop while the client's connected
      if (client.available()) {             // if there's bytes to read from the client,
        char c = client.read();             // read a byte, then
        Serial.write(c);                    // print it out the serial monitor
        if (c == '\n') {                    // if the byte is a newline character

          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0) {
            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line:
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:application/json");
            client.println();

            // TODO: Handle the response here. If we don't have an action and a number, tell the client it's an invalid response.
            action = getStringPartByNr(getRequest, '/', 1);
            actionPin = getStringPartByNr(getRequest, '/', 2).toInt();

            // set up the json here
            DynamicJsonBuffer jsonBuffer; // json buffer
            JsonObject& root = jsonBuffer.createObject();
            JsonArray& pins = root.createNestedArray("data");


            if (action.length() > 0 && action == "STATE" && actionPin > 0 && arrayIncludeElement(validPins,actionPin,validPinSize)){
              //client.println("You want the state of pin " + String(actionPin));
              pinMode(actionPin, OUTPUT);
              int val = digitalRead(actionPin);
              root["status"] = "ok";
              root["action"] = action;
              JsonObject& pin = pins.createNestedObject();
              pin["actionPin"] = actionPin;
              pin["val"] = val;
              root.printTo(client);
            } else if(action.length() > 0 && action == "STATE" && actionPin == 0) {
              root["status"] = "work in progress";
              root["action"] = action;

              /*
              for(int i=0;i<validPinSize;i++) {
                pinMode(validPins[i], OUTPUT);
                int val = digitalRead(validPins[i]);
                JsonObject& pin = pins.createNestedObject();
                pin["actionPin"] = validPins[i];
                pin["val"] = val;
              } */
              
              root.printTo(client);
            } else if (action.length() > 0 && action == "ON" && actionPin > 0 && arrayIncludeElement(validPins,actionPin,validPinSize)){
              //client.println("You want to turn on pin " + String(actionPin));
              pinMode(actionPin, OUTPUT);
              digitalWrite(actionPin,HIGH);
              int val = digitalRead(actionPin);
              root["status"] = "ok";
              root["action"] = action;
              JsonObject& pin = pins.createNestedObject();
              pin["actionPin"] = actionPin;
              pin["val"] = val;
              root.printTo(client);
            } else if (action.length() > 0 && action == "OFF" && actionPin > 0 && arrayIncludeElement(validPins,actionPin,validPinSize)){
              //client.println("You want to turn off pin " + String(actionPin));
              pinMode(actionPin, OUTPUT);
              digitalWrite(actionPin,LOW);
              int val = digitalRead(actionPin);
              root["status"] = "ok";
              root["action"] = action;
              JsonObject& pin = pins.createNestedObject();
              pin["actionPin"] = actionPin;
              pin["val"] = val;
              root.printTo(client);
            } else if (action.length() > 0 && action == "BUZZ" && actionPin > 0 && arrayIncludeElement(validPins,actionPin,validPinSize)){
              //client.println("You want to buzz pin " + String(actionPin));
              pinMode(actionPin, OUTPUT);
              digitalWrite(actionPin,HIGH);
              int val = digitalRead(actionPin);
              root["status"] = "ok";
              root["action"] = action;
              JsonObject& pin = pins.createNestedObject();
              pin["actionPin"] = actionPin;
              pin["val"] = val;
              root.printTo(client);
            } else { 
              root["status"] = "fail";
              root.printTo(client);
            } 
            
            
            // The HTTP response ends with another blank line:
            client.println();

            // end the connection to the client;
            client.stop();
            Serial.println("client disonnected");

            // stop a buzz if needed
            if (action.length() > 0 && action == "BUZZ" && actionPin > 0) {
              delay(5000);
              digitalWrite(actionPin,0);
            }
            
    
            // break out of the while loop:
            break;
          }
          else {      // if you got a newline, then clear currentLine:
            if(currentLine.startsWith("GET ")) {
              getRequest = currentLine;
            }
            currentLine = "";
          }
        }
        else if (c != '\r') {    // if you got anything else but a carriage return character,
          currentLine += c;      // add it to the end of the currentLine
        }

        /*
        // Check to see if the client request was "GET /H" or "GET /L":
        if (action.length() > 0 && currentLine.endsWith("/")) { 
          actionPin = currentLine.substring(0,currentLine.length()-2).toInt();
        } else if (currentLine.endsWith("GET /STATE/")) {
          action = "STATE";
          subLength = 11;
        } else if (currentLine.endsWith("GET /BUZZ/")) {
          action = "BUZZ";    
          subLength = 10;
        } else if (currentLine.endsWith("GET /ON/")) {
          action = "ON";
          subLength = 8;
        } else if (currentLine.endsWith("GET /OFF/")) {
          action = "OFF";
          subLength = 9;
        } */
      }
    }
  }
}

void printWifiStatus() {
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your WiFi shield's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
  // print where to go in a browser:
  Serial.print("To see this page in action, open a browser to http://");
  Serial.println(ip);
}

// tool to tokenize
// http://arduino.stackexchange.com/questions/1013/how-do-i-split-an-incoming-string
// spliting a string and return the part nr index split by separator
String getStringPartByNr(String data, char separator, int index) {
    int stringData = 0;        //variable to count data part nr 
    String dataPart = "";      //variable to hole the return text

    for(int i = 0; i<data.length()-1; i++) {    //Walk through the text one letter at a time

        if(data[i]==separator) {
            //Count the number of times separator character appears in the text
            stringData++;

        }else if(stringData==index) {
            //get the text when separator is the rignt one
            dataPart.concat(data[i]);

        }else if(stringData>index) {
            //return text and stop if the next separator appears - to save CPU-time
            return dataPart;
            break;

        }

    }
    //return text if this is the last part
    return dataPart;
}

 boolean arrayIncludeElement(int array[], int element, int arraySize) {
 for (int i = 0; i < arraySize; i++) {
      if (array[i] == element) {
          return true;
      }
    }
  return false;
 }
