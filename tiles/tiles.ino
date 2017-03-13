#include <Arduino.h>
#include <Wire.h>
#include <Brief.h>

#define EEPROM_ADDRESS_0 (byte)0x50 // 1010000

#define A_PIN 2
#define B_PIN 3
#define C_PIN 4
#define D_PIN 5

byte readEEPROM(unsigned int address) {
  Wire.beginTransmission(EEPROM_ADDRESS_0); // e.g. 0x50
  Wire.write(address);
  Wire.endTransmission();
  Wire.requestFrom(EEPROM_ADDRESS_0, (byte)1);
  if (Wire.available()) return Wire.read();
  return 0;
}

void writeEEPROM(unsigned int address, byte data) {
  Serial.print("Write ");
  Serial.print(address);
  Serial.print(" -> ");
  Serial.println(data);
  Wire.beginTransmission(EEPROM_ADDRESS_0); // e.g. 0x50
  Wire.write(address);
  Wire.write(data);
  Wire.endTransmission();
}

void selectTile(int num) {
  Serial.print("Select ");
  Serial.println(num);
  digitalWrite(A_PIN, (num & 1) != 0 ? HIGH : LOW);
  digitalWrite(B_PIN, (num & 2) != 0 ? HIGH : LOW);
  digitalWrite(C_PIN, (num & 4) != 0 ? HIGH : LOW);
  digitalWrite(D_PIN, (num & 8) != 0 ? HIGH : LOW);
}

byte readTile(int addr) {
  Serial.print("  Read: ");
  byte b = readEEPROM(addr);
  Serial.println(b);
  return b;
}

void writeTile(int addr, byte data) {
  writeEEPROM(addr, data);
}

void briefSelectTile()
{
  int num = brief::pop();
  selectTile(num);
}

void briefReadTile()
{
  int addr = brief::pop();
  brief::push(readTile(addr));
}

void briefWriteTile()
{
  int data = brief::pop();
  int addr = brief::pop();
  writeTile(addr, data);
}

void setup()
{
  // brief::setup(19200);

  Wire.begin();
  Serial.begin(19200);

  pinMode(A_PIN, OUTPUT);
  pinMode(B_PIN, OUTPUT);
  pinMode(C_PIN, OUTPUT);
  pinMode(D_PIN, OUTPUT);
    
  brief::bind(100, briefSelectTile);
  brief::bind(101, briefReadTile);
  brief::bind(102, briefWriteTile);

  /*
  Serial.println("Test Writing -----------------------------------");
  for (int i = 0; i < 16; i++) {
    selectTile(i);
    writeTile(0, i);
  }
  //*/

  //
  Serial.println("Test Reading -----------------------------------");
  for (int i = 0; i < 16; i++) {
    selectTile(i);
    readTile(0);
  }
  //*/
}

void loop()
{
  // brief::loop();
}
