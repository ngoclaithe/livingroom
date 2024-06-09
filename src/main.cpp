#include <SoftwareSerial.h>
#include "OneButton.h"
#include <DHT.h>

#define DHTTYPE DHT11

String local_add = "0xbb";
String rev_add = "0xff";
int nutnhan1 = 2;
const int lightPin = 8;
const int dhtPin = 3;

bool lightState = false;
float temperature = 0.0;
float humidity = 0.0;

OneButton button1(nutnhan1, true);
SoftwareSerial zigbeeSerial(11, 10);
DHT dht(dhtPin, DHTTYPE);

void changeLightState();
void readSensor();
void sendStatus();
void receiveCommand();

void setup() {
  Serial.begin(9600);
  zigbeeSerial.begin(9600);
  pinMode(lightPin, OUTPUT);
  dht.begin();

  button1.attachClick(changeLightState);
  Serial.println("Sender is ready.");
}

void loop() {
  button1.tick();
  receiveCommand();
  delay(10);

  static unsigned long lastSensorRead = 0;
  static unsigned long lastStatusSend = 0;
  unsigned long currentMillis = millis();
  if (currentMillis - lastSensorRead >= 1000) {
    lastSensorRead = currentMillis;
    readSensor();
  }

  if (currentMillis - lastStatusSend >= 1000) {
    lastStatusSend = currentMillis;
    sendStatus();
  }
}

void changeLightState() {
  lightState = !lightState;
  digitalWrite(lightPin, lightState ? HIGH : LOW);
  Serial.println("Button pressed, light state: " + String(lightState ? "ON" : "OFF"));
}

void readSensor() {
  temperature = dht.readTemperature();
  humidity = dht.readHumidity();
}

void sendStatus() {
  String message = String("Light: ") + (lightState ? "ON" : "OFF") +
                   ", Temp: " + String(temperature) + "C" +
                   ", Humidity: " + String(humidity) + "%" +
                   " From Add: " + local_add;
  zigbeeSerial.println(message);
  Serial.println("Sent: " + message);
}

void receiveCommand() {
  if (zigbeeSerial.available()) {
    String command = zigbeeSerial.readStringUntil('\n');
    command.trim();
    Serial.println("Received command: " + command);
    if (command == "ON") {
      lightState = true;
      digitalWrite(lightPin, HIGH);
    } else if (command == "OFF") {
      lightState = false;
      digitalWrite(lightPin, LOW);
    }
  }
}
