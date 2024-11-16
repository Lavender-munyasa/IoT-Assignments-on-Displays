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
#define INA_PIN 38
#define INB_PIN 35

// Buzzer setup
#define BUZZER_PIN 5

// I2C setup
#define I2C_SCL 2
#define I2C_SDA 42

// Keypad setup
#define ROWS 4
#define COLS 4
char keys[ROWS][COLS] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};
byte rowPins[ROWS] = {11, 9, 10, 1};  // GPIOs for rows
byte colPins[COLS] = {34, 21, 14, 12};  // GPIOs for columns
Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

// Variables
int targetTemperature = 30;
int criticalTemperature = 39;
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
    if (isnan(currentTemperature)) {
        Serial.println("Failed to read from DHT sensor!");
        return;
    }

    char key = keypad.getKey();
    handleKeypadInput(key);

    const char* fanStatus = (currentTemperature > targetTemperature) ? "ON" : "OFF";
    updateDisplay(currentTemperature, targetTemperature, fanStatus);

    if (millis() - lastFanCheckTime >= fanCheckInterval) {
        controlFan(currentTemperature);
        lastFanCheckTime = millis();
    }

    if (currentTemperature > criticalTemperature) {
        digitalWrite(BUZZER_PIN, HIGH);
    } else {
        digitalWrite(BUZZER_PIN, LOW);
    }
}

void handleKeypadInput(char key) {
    if (key) {
        if (key >= '0' && key <= '9') {
            targetTemperature = (targetTemperature * 10) + (key - '0');
        } else if (key == '#') {
            Serial.print("Target Temp Set: ");
            Serial.println(targetTemperature);
        } else if (key == '*') {
            targetTemperature = 0;
        }
        delay(50);  // Simple debounce
    }
}

void updateDisplay(float currentTemperature, float targetTemp, const char* fanStatus) {
    display.clearDisplay();
    display.setCursor(0, 0);
    display.print("Current Temp: ");
    display.print(currentTemperature);
    display.print(" C");
    display.setCursor(0, 20);
    display.print("Target Temp: ");
    display.print(targetTemp);
    display.print(" C");
    display.setCursor(0, 40);
    display.print("Fan: ");
    display.print(fanStatus);
    display.display();
}

void controlFan(float currentTemperature) {
    if (currentTemperature > targetTemperature) {
        digitalWrite(INA_PIN, LOW);
        digitalWrite(INB_PIN, HIGH);
        Serial.println("Fan: ON");
    } else {
        digitalWrite(INA_PIN, LOW);
        digitalWrite(INB_PIN, LOW);
        Serial.println("Fan: OFF");
    }
}
