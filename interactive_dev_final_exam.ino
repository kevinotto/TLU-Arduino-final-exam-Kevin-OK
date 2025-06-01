// Final Arduino exam project
// By Kevin Otto Koit
// IFI7340.DT 2025 Spring, HCI, TLU

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <DHT.h>
#include <Wire.h>

// === OLED configuration ===
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// === Pin assignments for input and output components ===
#define DHTPIN 2
#define DHTTYPE DHT22
#define MQ135PIN A3
#define BUTTON_PIN 3
#define BUZZER_PIN 6
#define FAN_PIN 5

// === DHT configuration ===
DHT dht(DHTPIN, DHTTYPE);

// === Thresholds for the sensors ===
float highTempThreshold = 30.0;
float lowHumidityThreshold = 40.0;
float highHumidityThreshold = 70.0;
int airQualityThreshold = 200;

// === Alert system configuration ===
bool alertActive = false; //set false so alarm doesn't start automatically at start
bool snoozed = false; //set false so alarm won't snooze automatically at start
unsigned long snoozeStartTime = 0; //value set 0 so the snooze starts instantly when button is being pressed
const unsigned long snoozeDuration = 30000; // snooze duration is set 30 seconds

// === Timing update ===
unsigned long lastUpdateTime = 0;
const unsigned long updateInterval = 2000; // set 2 seconds so sensors' data read is more accurate

// === Sensor readings ===
float temp = 0;
float humidity = 0;
int airQuality = 0;

// === Icon for the alert display ===
const unsigned char alert_icon [] PROGMEM = {
  0b00011000,
  0b00111100,
  0b01111110,
  0b01111110,
  0b01111110,
  0b00111100,
  0b00011000,
  0b00000000
};

void setup() {
  Serial.begin(9600);
  dht.begin();

  pinMode(MQ135PIN, INPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(FAN_PIN, OUTPUT);

// === In case if display startup fails ===
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("OLED init failed");
    while (true); // Freeze
  }

  display.clearDisplay();
  display.display();
}

void loop() {
  // === Alert sound snooze when buttons is being pressed ===
  if (digitalRead(BUTTON_PIN) == LOW && alertActive && !snoozed) {
    noTone(BUZZER_PIN);
    alertActive = false;
    snoozed = true;
    snoozeStartTime = millis();
    Serial.println("Alarm snoozed for 30 seconds ");
    delay(50);
  }

  // === Alert snooze end after 30 seconds ===
  if (snoozed && millis() - snoozeStartTime >= snoozeDuration) {
    snoozed = false;
    Serial.println("Alarm snooze ended ");
  }

  // === Display update, reading sensor data periodically ===
  if (millis() - lastUpdateTime >= updateInterval) {
    lastUpdateTime = millis();

    // === Readings of the sensors ===
    temp = dht.readTemperature();
    humidity = dht.readHumidity();
    airQuality = analogRead(MQ135PIN);

    // === For server console display ===
    Serial.print("Temp: "); Serial.print(temp); Serial.print(" °C, "); //displays the temperature readings
    Serial.print("Humidity: "); Serial.print(humidity); Serial.print(" %, "); //displays the humidity readings
    Serial.print("Air quality: "); Serial.println(airQuality); //displays the air quality readings

    // === Display update ===
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE); //sets the text color to white
    display.setCursor(0, 0);
    display.print("Temp: "); display.print(temp); display.println(" C"); //displays the temperature readings
    display.print("Humidity: "); display.print(humidity); display.println(" %"); //displays the humidity readings
    display.print("Air Quality: "); display.println(airQuality); //displays the air quality readings

    int alertY = 48;

    // === Alerts on display ===
    bool badAir = airQuality > airQualityThreshold; //if the air quality value is higher than the threshold, the air quality will be considered bad
    bool badHumidity = humidity < lowHumidityThreshold || humidity > highHumidityThreshold; //if the humidity value is higher than the threshold, the humidity will be considered bad
    bool tempTooHigh = temp > highTempThreshold; //if the temperature value is higher than the threshold, the temperature will be considered high

    if (tempTooHigh) {
      display.drawBitmap(0, alertY, alert_icon, 8, 8, SSD1306_WHITE);
      display.setCursor(10, alertY);
      display.println("Temp too high! "); //for display
      Serial.print("Temp too high! "); //for server
      alertY += 10;
    }

    if (badAir) {
      display.drawBitmap(0, alertY, alert_icon, 8, 8, SSD1306_WHITE);
      display.setCursor(10, alertY);
      display.println("Bad air quality! "); //for display
      Serial.print("Bad air quality! "); //for server
      alertY += 10;
    }

    if (badHumidity) {
      display.drawBitmap(0, alertY, alert_icon, 8, 8, SSD1306_WHITE);
      display.setCursor(10, alertY);
      display.println("Bad humidity!"); //for display
      Serial.print("Bad humidity!"); // for server
    }

    display.display();

    // === Alert logic ===
    if ((badAir || badHumidity) && !snoozed) {
      tone(BUZZER_PIN, 4000); //activates piezo buzzer on 4 KHz if air quality and humidity values exceeds the threshold
      alertActive = true;
    } else {
      noTone(BUZZER_PIN); //piezo buzzer is disabled if there's no alert
      alertActive = false;
    }

    // === Fan control ===
    if (tempTooHigh) {
      digitalWrite(FAN_PIN, HIGH); //activates fan
    } else {
      digitalWrite(FAN_PIN, LOW); //fan is disabled if the temperature is not exceeding the threshold
    }
  }
}
