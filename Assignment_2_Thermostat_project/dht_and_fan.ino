#include <DHT.h>
#include <Wire.h>
#include <Adafruit_SSD1306.h>
#include <Keypad.h>

// DHT22 sensor setup
#define DHTPIN 39
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

// OLED display setup
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// Fan setup
#define INA_PIN 38  //D3
#define INB_PIN 35

// Buzzer and OLED setup
#define BUZZER_PIN 5
#define I2C_SCL 22 // Standard SCL for ESP32
#define I2C_SDA 21 // Standard SDA for ESP32

// Keypad setup
#define ROWS 4 // four rows
#define COLS 4 // four columns
char keys[ROWS][COLS] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};
byte rowPins[ROWS] = {11, 9, 10, 1}; // connect to the row pinouts of the keypad
byte colPins[COLS] = {34, 21, 14, 12}; // connect to the column pinouts of the keypad

Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

// Variables
int targetTemperature = 30; // Default target temperature
int criticalTemperature = 39; // Critical temperature threshold
unsigned long lastFanCheckTime = 0;
const unsigned long fanCheckInterval = 2000;

void setup() {
    Serial.begin(115200);
    dht.begin();
    Wire.begin(I2C_SDA, I2C_SCL);
    display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
    pinMode(BUZZER_PIN, OUTPUT);
    pinMode(INA_PIN, OUTPUT);
    pinMode(INB_PIN, OUTPUT);
    digitalWrite(BUZZER_PIN, LOW);
    digitalWrite(INA_PIN, LOW);
    digitalWrite(INB_PIN, LOW);

    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
}

void loop() {
    float currentTemperature = dht.readTemperature();
    char key = keypad.getKey();
    handleKeypadInput(key);

    // Display current temperature and target temperature
    displayCurrentTemperature(currentTemperature);
    displayTargetTemperature(targetTemperature);

    // Control fan without blocking code
    unsigned long currentTime = millis();
    if (currentTime - lastFanCheckTime >= fanCheckInterval) {
        controlFan(currentTemperature);
        lastFanCheckTime = currentTime;
    }

    // Critical temperature alert
    if (currentTemperature > criticalTemperature) {
        digitalWrite(BUZZER_PIN, HIGH);
    } else {
        digitalWrite(BUZZER_PIN, LOW);
    }
}

// Handle keypad input for setting the target temperature
void handleKeypadInput(char key) {
    if (key) {
        if (key >= '0' && key <= '9') {
            targetTemperature = (targetTemperature * 10) + (key - '0');
        } else if (key == '#') { // Confirm target temperature
            Serial.print("Target Temp Set: ");
            Serial.println(targetTemperature);
            displayTargetTemperature(targetTemperature);
        } else if (key == '*') { // Reset target temperature
            targetTemperature = 0;
        }
        delay(50); // Debounce
    }
}

// Display current temperature
void displayCurrentTemperature(float currentTemperature) {
    display.clearDisplay();
    display.setCursor(0, 0);
    display.print("Current Temp: ");
    display.print(currentTemperature);
    display.print(" C");
    display.display();
}

// Display target temperature
void displayTargetTemperature(float targetTemp) {
    display.setCursor(0, 20);
    display.print("Target Temp: ");
    display.print(targetTemp);
    display.print(" C");
    display.display();
}

// Fan control based on temperature
void controlFan(float currentTemperature) {
    if (currentTemperature > targetTemperature) {
        digitalWrite(INA_PIN, LOW);
        digitalWrite(INB_PIN, HIGH);
        displayFanStatus("ON");
        Serial.println("Fan: ON");
    } else {
        digitalWrite(INA_PIN, LOW);
        digitalWrite(INB_PIN, LOW);
        displayFanStatus("OFF");
        Serial.println("Fan: OFF");
    }
}

// Display fan status
void displayFanStatus(const char* status) {
    display.setCursor(0, 40);
    display.print("Fan: ");
    display.print(status);
    display.display();
}
