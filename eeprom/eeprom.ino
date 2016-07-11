#include <Arduino.h>
#include <Wire.h>

#define EEPROM_ADDRESS_0 (byte)0x50 // 1010000
#define EEPROM_ADDRESS_1 (byte)0x51 // 1010001
#define RESET_PIN 0
#define PULSE_PIN 1

#define RED   12
#define GREEN 14
#define BLUE  15

void led(bool red, bool green, bool blue) {
  analogWrite(RED,   red   ? 254 : 255);
  analogWrite(GREEN, green ? 251 : 255);
  analogWrite(BLUE,  blue  ? 251 : 255);
  delay(50);
}

void K() { led(false, false, false); }
void R() { led( true, false, false); }
void G() { led(false,  true, false); }
void B() { led(false, false,  true); }
void C() { led(false,  true,  true); }
void M() { led( true, false,  true); }
void Y() { led( true,  true, false); }
void W() { led( true,  true,  true); }

void writeEEPROM(byte device, unsigned int address, byte data) {
  Wire.beginTransmission(device); // e.g. 0x50
  Wire.write(address);
  Wire.write(data);
  Wire.endTransmission();
  delay(10);
}

byte readEEPROM(byte device, unsigned int address) {
  Wire.beginTransmission(device); // e.g. 0x50
  Wire.write(address);
  Wire.endTransmission();
  delay(10);
  Wire.requestFrom(device, (byte)1);
  byte data = 0;
  if (Wire.available()) {
    data = Wire.read();
    Serial.println(data, HEX);
    //switch (data) {
    //  case 0x0a: R(); break;
    //  case 0x0b: G(); break;
    //  case 0x0c: B(); break;
    //  case 0x0d: C(); break;
    //  case 0x0e: M(); break;
    //  case 0x0f: Y(); break;
    //}
  } else {
    Serial.println("?");
    //K();
  }
  return data;
}

void writeToChip(byte device, byte value) {
  Serial.print("Write to EEPROM ");
  Serial.print(device);
  Serial.print(" -> ");
  Serial.println(value);
  writeEEPROM(device, 0, value);
}

void readFromChip(byte device) {
  Serial.print("Read from EEPROM ");
  Serial.println(device);
  readEEPROM(device, 0);
}

void reset() {
  Serial.println("Reset");
  digitalWrite(RESET_PIN, LOW);
  delay(10);
  digitalWrite(RESET_PIN, HIGH);
}

void pulse() {
  Serial.println("Pulse");
  digitalWrite(PULSE_PIN, LOW);
  delay(10);
  digitalWrite(PULSE_PIN, HIGH);  
  delay(490);
}

void readFromBoard() {
  Serial.print("Chip 0      Main: ");
  if (readEEPROM(EEPROM_ADDRESS_0, 0)) {
    Serial.print("Chip 1 Parameter: ");
    readEEPROM(EEPROM_ADDRESS_1, 0);
    pulse();
    readFromBoard();
  }
}

void readSequence() {
  //K();
  reset();
  delay(1000);
  readFromBoard();
}

void setup()
{
  Serial.begin(9600);
  Wire.begin();
  pinMode(RED,   OUTPUT);
  pinMode(GREEN, OUTPUT);
  pinMode(BLUE,  OUTPUT);
  K();
  pinMode(RESET_PIN, OUTPUT);
  pinMode(PULSE_PIN, OUTPUT);
  digitalWrite(RESET_PIN, HIGH);
  digitalWrite(PULSE_PIN, HIGH);
}

int v = 0x4C;

void loop() {
  //readSequence(); // test
  //readFromBoard();
  //Serial.print(" ");
  //readFromChip();
  //Serial.println("");
  //Serial.println("");
  //Serial.println("");
  //delay(4000);
  
  char c = Serial.read();
  if (c == 'p') {
    Serial.print("Programming: ");
    Serial.println(v);
    writeToChip(EEPROM_ADDRESS_0, v);
  }
  if (c == 'r') {
    Serial.print("Reading: ");
    readFromChip(EEPROM_ADDRESS_0);
  }
  if (c == 'n') {
    v++;
    Serial.print("Value: ");
    Serial.println(v);
  }
  if (c == 'b') {
    v--;
    Serial.print("Value: ");
    Serial.println(v);
  }
  if (c == 's') {
    readSequence(); // test    
  }
}
