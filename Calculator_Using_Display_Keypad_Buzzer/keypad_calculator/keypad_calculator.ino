#include <Keypad.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// I2C pins for the ESP32-S2
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

byte rowPins[ROWS] = {11, 9, 10, 1}; // GPIO connections
byte colPins[COLS] = {34, 21, 14, 12}; // GPIO connections

Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

int buzzerPin = 5; // Buzzer pin

// Calculator variables
String input = "";
String operand1 = "", operand2 = "";
char operation = 0;
bool resultCalculated = false;  // Flag to indicate result calculation

void setup() {
  Serial.begin(115200);
  
  // Initialize I2C and OLED display
  Wire.begin(SDA_PIN, SCL_PIN);
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { 
    Serial.println(F("SSD1306 allocation failed"));
    for (;;);
  }

  display.display();
  delay(2000); // Splash screen delay
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
    displayInput(); // Update display after each key press
  }
}

void handleKeyPress(char key) {
  if (key >= '0' && key <= '9') { // Number keys
    if (resultCalculated) {  // Start fresh if a number is pressed after a result
      clearCalculator();
    }
    input += key;
  } else if (key == 'C') { // Clear
    clearCalculator();
  } else if (key == '=' && operation) { // Evaluate when '=' is pressed
    operand2 = input;
    calculateResult();
  } else if (key == '+' || key == '-' || key == '*' || key == '/') { 
    // Operation keys
    if (!input.isEmpty() && !resultCalculated) {
      operand1 = input;
      operation = key;
      input = "";  // Clear input for the second operand
    } else if (resultCalculated) {
      // Allow continued operations with result as operand1
      operation = key;
      input = "";
      resultCalculated = false; // Reset the result flag
    }
  }
}

void clearCalculator() {
  input = "";
  operand1 = "";
  operand2 = "";
  operation = 0;
  resultCalculated = false;
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
  else if (operation == '/') result = (num2 != 0) ? num1 / num2 : 0; // Prevent division by zero

  // Store the result and mark it for display
  input = String(result, 2);
  operand1 = input;  // Keep result in operand1 for further operations
  operand2 = "";     // Clear operand2
  operation = 0;     // Clear operation
  resultCalculated = true;  // Set flag to prevent additional operations until reset
  displayInput();    // Display the final result
}

void displayInput() {
  display.clearDisplay();
  display.setCursor(0, 0);

  if (resultCalculated) { 
    // Display only the result when calculated
    display.print(input);
  } else {
    // Show current inputs if no result yet
    display.print(operand1);
    if (operation) display.print(" ");
    display.print(operation);
    display.print(" ");
    display.print(input);
  }

  display.display();
}
