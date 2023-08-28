#include <Encoder.h>
#include <SoftwareSerial.h>

// Define the pins for the L298N motor driver
const int enA = 7;
const int in1 = 5;
const int in2 = 6;

// Define the pins for the rotary encoder
const int encoderA = 8;
const int encoderB = 9;
Encoder myEncoder(encoderA, encoderB);

int solenoidPin = 2;
int ledbluePin = 3;
int ledredPin = 4;


// Define the Bluetooth module RX and TX pins
SoftwareSerial bluetoothSerial(0, 1);

// Define the variables to store the desired length, quantity, and count
uint32_t length = 0;
uint32_t quantity = 0;
int count = 0;

void resetEncoder() {
  myEncoder.write(0);
}

void setup() {
  // Initialize the serial communication
  Serial.begin(9600);
  bluetoothSerial.begin(9600);

  // Set the motor driver pins as outputs
  pinMode(enA, OUTPUT);
  pinMode(in1, OUTPUT);
  pinMode(in2, OUTPUT);

  pinMode(solenoidPin, OUTPUT);
  pinMode(ledbluePin, OUTPUT);
  pinMode(ledredPin, OUTPUT);


  // Set the initial position of the encoder to zero
  myEncoder.write(0);

  Serial.println("Waiting for data...");
  digitalWrite(ledbluePin, HIGH);
}

void loop() {
  static bool newDataReceived = false;
  if (newDataReceived && Serial.available()) {
    // If new data has been received and the DONE text has been printed,
    // read and process new data
    Serial.read();
    newDataReceived = false;
  }
  if (bluetoothSerial.available() >= 8 && !newDataReceived) {
    // Read the 8-byte number from the Bluetooth module
    uint64_t combinedValue = 0;
    for (int i = 0; i < 8; i++) {
      combinedValue <<= 8;
      combinedValue |= bluetoothSerial.read();
    }

    // Extract the quantity value and length value
    quantity = (combinedValue >> 32) & 0xFFFFFFFF;
    length = combinedValue & 0xFFFFFFFF;

    // Check if quantity or length value is zero
    if (quantity == 0 || length == 0) {
      Serial.println("Error: Received data with zero quantity or length");
      Serial.println("Input new values for length and quantity:");
      return; // skip the rest of the loop and wait for new data
    }

    // Print the values on the serial monitor
    Serial.print("Quantity: ");
    Serial.println(quantity >> 24);
    Serial.print("Length Value: ");
    float value = length / 16777216.0; // Calculate the value
    Serial.print(value, 2);

    digitalWrite(ledredPin, HIGH);
    digitalWrite(ledbluePin, LOW);

    for (int i = 0; i < quantity / 16777216; i++) {

      // Wait 1 seconds before starting the motor
      delay(1000);
      resetEncoder();
      
      // Activate the motor
      Serial.println("Motor ON");
      digitalWrite(in1, LOW);
      digitalWrite(in2, HIGH);
      analogWrite(enA, 1000);

      // Measure the length with the rotary encoder
      long currentLength = 0;
      long previousPosition = -999;
      while (currentLength < length / 16777216) {
        long newPosition = myEncoder.read();
        if (newPosition != previousPosition) {
          currentLength = abs(newPosition);
          Serial.print("Current length: ");
          Serial.print(currentLength);
          Serial.println(" mm");
          previousPosition = newPosition;
        }
      }
      // Reset encoder position to zero
      myEncoder.write(0);


      // Stop the motor
      digitalWrite(in1, LOW);
      digitalWrite(in2, LOW);
      analogWrite(enA, LOW);
      Serial.println("Motor OFF");

      // Turn on the solenoid valve
      digitalWrite(solenoidPin, HIGH);
      Serial.println("Solenoid ON");

      //wait for 5 seconds
      delay(1000);

      //turn off the solenoid valve
      digitalWrite(solenoidPin, LOW);
      Serial.println("Solenoid OFF");

      //increment the count and reset the current position
      count++;
      currentLength = 0;

      Serial.print("Count: ");
      Serial.println(count);
    }

    // Print "DONE" on the serial monitor and set newDataReceived to true
    Serial.println("DONE");
    newDataReceived = true;
    // Reset the count to zero
    count = 0;

    // Turn off the LED
    digitalWrite(ledbluePin, HIGH);
    digitalWrite(ledredPin, LOW);

    // Prompt the user to enter new values
    Serial.println("Enter new values of quantity and length:");

    // Wait for 1 seconds
    delay(1000);

  }
}
