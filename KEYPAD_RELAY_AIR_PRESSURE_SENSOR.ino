#include <Keypad.h>

const byte ROWS = 4; // Four rows
const byte COLS = 4; // Four columns
char keys[ROWS][COLS] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};
byte rowPins[ROWS] = {9, 8, 7, 6}; // Connect to the row pinouts of the keypad
byte colPins[COLS] = {5, 4, 3, 2}; // Connect to the column pinouts of the keypad
Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

const int pressurePin = A1; // Analog pin for the pressure sensor
const float sensorMinValue = 0.0; // Minimum analog value from the sensor
const float sensorMaxValue = 1023.0; // Maximum analog value from the sensor
const float pressureMinValue = 0.0; // Minimum pressure value in psi
const float pressureMaxValue = 3.0; // Maximum pressure value in psi

String input = "";
const float conversionFactor = 0.1;

float enteredPressure = 0.0;

const int relayPin = 10;
boolean relayActive = false;

void setup() {
  Serial.begin(9600);
  pinMode(relayPin, OUTPUT);
  digitalWrite(relayPin, HIGH);
  Serial.println("Enter pressure required and press C to start:");
}

void loop() {
  char key = keypad.getKey();

  if (key) {
    if (key == 'C') {
      enteredPressure = input.toFloat();
      Serial.print("Entered pressure: ");
      Serial.print(enteredPressure, 2);
      Serial.println(" psi");
      input = "";
      relayActive = true; // Start the compressor motor
    } else if (key == 'D') {
      if (input.length() > 0) {
        input.remove(input.length() - 1);
        Serial.println("Current input: " + input);
      }
    } else if (key >= '0' && key <= '9') {
      input += key;
      Serial.println("Current input: " + input);
    } else if (key == 'A') {
      input += ".";
      Serial.println("Current input: " + input);
    }
  }

  if (relayActive) {
    delay(1000);
    int rawValue = analogRead(pressurePin);
    float pressureValue = mapFloat(rawValue, sensorMinValue, sensorMaxValue, pressureMinValue, pressureMaxValue);
    pressureValue = constrain(pressureValue, pressureMinValue, pressureMaxValue);

    Serial.print("Current pressure reading: ");
    Serial.print(pressureValue, 2);
    Serial.println(" psi");

    if (pressureValue >= enteredPressure) {
      digitalWrite(relayPin, HIGH); // Turn off the relay and stop the compressor
      relayActive = false;
      Serial.println("Required pressure achieved. Compressor stopped.");
      Serial.println("Enter new pressure value and press C to enter:");
    }else{
      digitalWrite(relayPin, LOW);
      }
    delay(1000);
  }
}

float mapFloat(float x, float inMin, float inMax, float outMin, float outMax) {
  return (x - inMin) * (outMax - outMin) / (inMax - inMin) + outMin;
}
