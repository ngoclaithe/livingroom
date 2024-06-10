#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <SoftwareSerial.h>
#include "OneButton.h"
#include <DHT.h>
#include <avr/wdt.h>

#define DHTTYPE DHT11

String local_add = "0xbb";
int nutnhan1 = 2;
const int lightPin = 8;
const int dhtPin = 3;
const int pirPin = 9;
const int buzzerPin = 13;
bool lightState = false;
float temperature = 0.0;
float humidity = 0.0;
int pirState = 0;

OneButton button1(nutnhan1, true);
SoftwareSerial zigbeeSerial(11, 10);
DHT dht(dhtPin, DHTTYPE);
LiquidCrystal_I2C lcd(0x27, 16, 2);

void changeLightState();
void readSensor();
void sendStatus();
void receiveCommand();

void setup() {
  Serial.begin(9600);
  zigbeeSerial.begin(9600);
  pinMode(lightPin, OUTPUT);
  pinMode(buzzerPin, OUTPUT);
  pinMode(pirPin, INPUT);
  dht.begin();
  lcd.init();
  lcd.backlight();
  digitalWrite(buzzerPin, LOW);
  button1.attachClick(changeLightState);
  Serial.println("Sender is ready.");
  wdt_enable(WDTO_2S);
}

void loop() {
  wdt_reset();
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
  pirState = digitalRead(pirPin);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Temp: ");
  lcd.print(temperature);
  lcd.print("C");
  lcd.setCursor(0, 1);
  lcd.print("Humidity: ");
  lcd.print(humidity);
  lcd.print("%");
}

void sendStatus() {
  String message = String("Light: ") + (lightState ? "ON" : "OFF") +
                   ", Temp: " + String(temperature) + "C" +
                   ", Humidity: " + String(humidity) + "%" +
                   ", PIR: " + String(pirState) +
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
    } else if (command == "BUZZON") {
      digitalWrite(buzzerPin, HIGH);
    } else if (command == "BUZZOFF") {
      digitalWrite(buzzerPin, LOW);
    }
  }
}
