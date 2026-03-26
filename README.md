# Arduino JSON API

A WiFi-enabled Arduino web server that exposes a JSON API for controlling digital pins over HTTP. Originally built for remote control of a door buzzer.

## Requirements

- Arduino board with [WiFi Shield 101](https://www.arduino.cc/en/Guide/ArduinoWiFiShield101)
- [ArduinoJson](https://arduinojson.org/) library (v5)
- [WiFi101](https://www.arduino.cc/en/Reference/WiFi101) library

## Setup

1. Open `arduino-json-api.ino` in the Arduino IDE
2. Update `ssid` and `pass` with your WiFi credentials
3. Adjust `validPins[]` if needed (some pins are reserved for the WiFi shield)
4. Upload to your board

The serial monitor (9600 baud) will display the board's IP address once connected.

## API

All endpoints use HTTP GET and return JSON responses.

### `GET /STATE/<pin>`

Read the current state of a pin.

```
GET /STATE/2
```
```json
{"status":"ok","action":"STATE","data":[{"pin":2,"value":0}]}
```

### `GET /STATE`

Read the state of all valid pins.

```
GET /STATE
```
```json
{"status":"ok","action":"STATE","data":[{"pin":1,"value":0},{"pin":2,"value":1},...]}
```

### `GET /ON/<pin>`

Set a pin HIGH.

```
GET /ON/2
```
```json
{"status":"ok","action":"ON","data":[{"pin":2,"value":1}]}
```

### `GET /OFF/<pin>`

Set a pin LOW.

```
GET /OFF/2
```
```json
{"status":"ok","action":"OFF","data":[{"pin":2,"value":0}]}
```

### `GET /BUZZ/<pin>`

Set a pin HIGH for 5 seconds, then automatically turn it LOW. The response is returned immediately (non-blocking).

```
GET /BUZZ/2
```
```json
{"status":"ok","action":"BUZZ","data":[{"pin":2,"value":1}]}
```

### Error Response

Invalid actions, missing pins, or disallowed pins return a 400 status:

```json
{"status":"error"}
```

## Valid Pins

By default, pins 1, 2, 3, 4, 6, 8, 9, 10, 11, 12, 13 are available. Pins 0, 5, and 7 are reserved for the WiFi shield. Edit `validPins[]` to change this.
