#include <Keypad.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <DHT.h>

#define DHTPIN 4 // Pin where the DHT22 is connected
#define DHTTYPE DHT22 // Define the type of DHT sensor
DHT dht(DHTPIN, DHTTYPE);

// Define the I2C pins for the ESP32-S2
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#define SDA_PIN 42
#define SCL_PIN 2
#define FAN_PIN 38 // Fan control pin

// Original Keypad configuration
const byte ROWS = 4;
const byte COLS = 4;
char keys[ROWS][COLS] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};

byte rowPins[ROWS] = {11, 9, 10, 1}; 
byte colPins[COLS] = {34, 21, 14, 12}; 

Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

float targetTemperature = 0; // Target temperature set by the user
bool fanStatus = false; // Fan status (on/off)

void setup() {
  Serial.begin(115200);
  
  // Initialize I2C communication
  Wire.begin(SDA_PIN, SCL_PIN);
  
  // Initialize OLED display
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3C for OLED
    Serial.println(F("SSD1306 allocation failed"));
    for (;;); // Loop here if the display doesn't initialize
  }
  
  display.display();
  delay(2000); // Initial splash screen
  display.clearDisplay();

  pinMode(FAN_PIN, OUTPUT); // Fan control pin setup
  dht.begin(); // Initialize DHT sensor
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.display();
}

void loop() {
  float currentTemperature = dht.readTemperature(); // Read the current temperature
  char key = keypad.getKey(); // Read the key

  // Display current temperature and target temperature
  display.clearDisplay();
  display.setCursor(0, 0);
  display.print("Current Temp: ");
  display.print(currentTemperature);
  display.print(" C");

  display.setCursor(0, 30);
  display.print("Target Temp: ");
  display.print(targetTemperature);
  display.print(" C");

  // Check if the fan should be turned on or off
  if (currentTemperature > targetTemperature) {
    digitalWrite(FAN_PIN, HIGH); // Turn on the fan
    fanStatus = true;
  } else {
    digitalWrite(FAN_PIN, LOW); // Turn off the fan
    fanStatus = false;
  }

  // Display fan status
  display.setCursor(0, 50);
  display.print("Fan: ");
  display.print(fanStatus ? "ON" : "OFF");
  display.display();

  // Prompt for setting target temperature
  display.setCursor(0, 0);
  display.print("Set Target Temp:");

  if (key) {
    if (key >= '0' && key <= '9') { // If the key is a number
      targetTemperature = targetTemperature * 10 + (key - '0'); // Update target temp
      display.clearDisplay();
      display.setCursor(0, 0);
      display.print("Target Temp: ");
      display.print(targetTemperature);
      display.display();
    } else if (key == '#') { // If the key is '#', confirm the target temperature
      Serial.print("Target Temperature Set: ");
      Serial.println(targetTemperature);
    } else if (key == 'C') { // Clear the target temperature
      targetTemperature = 0; // Reset
      display.clearDisplay();
    }
  }

  delay(100); // Small delay to debounce the keypad
}

