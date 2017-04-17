#include <Arduino.h>
#include <Wire.h>

void setup() {
  Serial.begin(9600);
  Wire.begin();
}

void loop() {
  Serial.println("Writing 42");
  Wire.beginTransmission(80); // 1010000
  Wire.write(0); // address low
  Wire.write(42); // data
  Wire.endTransmission();

  delay(100);
  
  Serial.print("Reading: ");
  Wire.beginTransmission(80); // 1010000
  Wire.write(0); // address low
  Wire.endTransmission();
  Wire.requestFrom(80, 1);
  if (Wire.available()) {
    byte data = Wire.read();
    Serial.println(data, DEC);
  } else {
    Serial.println("???");
  }

  delay(1000);
}
