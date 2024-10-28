#include <Keypad.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Defining the I2C pins for the ESP32-S2
#define SDA_PIN 42
#define SCL_PIN 2
#define INA_PIN 38  //D3
#define INB_PIN 35

// Keypad configuration
const byte ROWS = 4;
const byte COLS = 4;
char keys[ROWS][COLS] = {
  {'1', '2', '3', '+'},
  {'4', '5', '6', '-'},
  {'7', '8', '9', '*'},
  {'C', '0', '=', '/'}
};

byte rowPins[ROWS] = {11, 9, 10, 1}; // Selected GPIO connections
byte colPins[COLS] = {34, 21, 14, 12}; // Selected GPIO connections

Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

int buzzerPin = 5; // Buzzer pin

// Calculator variables
String input = "";
String operand1 = "", operand2 = "";
char operation = 0;
bool calculating = false;
bool resultDisplayed = false;  

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

  pinMode(buzzerPin, OUTPUT); // Buzzer setup
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.display();
}

void loop() {
  char key = keypad.getKey();

  if (key) {
    tone(buzzerPin, 1000, 100); // Sound feedback
    handleKeyPress(key);
    if (!resultDisplayed || key != '=') { // Update display only if result has not been shown yet to try stop double printing
      displayInput();
    }
  }
}

void handleKeyPress(char key) {
  if (key >= '0' && key <= '9') { // Number keys
    if (resultDisplayed) {  // Clear display if new input after result
      clearCalculator();
      resultDisplayed = false;
    }
    input += key;
  } else if (key == 'C') { // Clear
    clearCalculator();
  } else if (key == '=' && operation) { // Evaluate
    operand2 = input;
    calculateResult();
  } else if (key == '+' || key == '-' || key == '*' || key == '/') { // Operation
    if (!operation && !input.isEmpty()) {
      operand1 = input;
      operation = key;
      input = "";
    }
  }
}

void clearCalculator() {
  input = "";
  operand1 = "";
  operand2 = "";
  operation = 0;
  calculating = false;
  resultDisplayed = false;
  display.clearDisplay();
  display.setCursor(0, 0);
}

void calculateResult() {
  float num1 = operand1.toFloat();
  float num2 = operand2.toFloat();
  float result = 0.0;

  if (operation == '+') result = num1 + num2;
  else if (operation == '-') result = num1 - num2;
  else if (operation == '*') result = num1 * num2;
  else if (operation == '/') result = num2 != 0 ? num1 / num2 : 0; // Prevent division by zero/optional

  input = String(result, 2); // Display result with 2 decimal places
  operand1 = input;
  operation = 0;
  operand2 = "";
  resultDisplayed = true;  // Set flag to indicate result was displayed
}

void displayInput() {
  display.clearDisplay();
  display.setCursor(0, 0);
  display.print(operand1);
  if (operation) display.print(" ");
  display.print(operation);
  display.print(" ");
  display.print(input);
  display.display();
}

