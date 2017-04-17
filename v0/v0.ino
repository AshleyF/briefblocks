#include <Arduino.h>
#include <Wire.h>
#include <Brief.h>

#define EEPROM_ADDRESS_0 (byte)0x50 // 1010000
#define EEPROM_ADDRESS_1 (byte)0x51 // 1010001
#define EEPROM_ADDRESS_2 (byte)0x52 // 1010010
#define EEPROM_ADDRESS_3 (byte)0x53 // 1010011

#define A_PIN 2
#define B_PIN 3
#define C_PIN 4
#define D_PIN 5

unsigned int tileDevice(int num) {
  switch (num) {
    case 0: return EEPROM_ADDRESS_0;
    case 1: return EEPROM_ADDRESS_1;
    case 2: return EEPROM_ADDRESS_2;
    case 3: return EEPROM_ADDRESS_3;
    default: return 0; // TODO: error if not 0 >= num <= 3
  }
}

byte readEEPROM(byte device, unsigned int address) {
  Wire.beginTransmission(device); // e.g. 0x50
  Wire.write(address);
  Wire.endTransmission();
  Wire.requestFrom(device, (byte)1);
  if (Wire.available()) return Wire.read();
  return 0;
}

void writeEEPROM(byte device, unsigned int address, byte data) {
  Wire.beginTransmission(device); // e.g. 0x50
  Wire.write(address);
  Wire.write(data);
  Wire.endTransmission();
}

void selectBank(int num) {
  // Serial.print("Bank ");
  // Serial.println(num);
  digitalWrite(A_PIN, (num & 1) != 0 ? HIGH : LOW);
  digitalWrite(B_PIN, (num & 2) != 0 ? HIGH : LOW);
  digitalWrite(C_PIN, (num & 4) != 0 ? HIGH : LOW);
  digitalWrite(D_PIN, (num & 8) != 0 ? HIGH : LOW);
}

byte readTile(int num, int addr) {
  // Serial.print("  Tile ");
  // Serial.print(num);
  // Serial.print(": ");
  byte b = readEEPROM(tileDevice(num), addr);
  // Serial.println(b);
  return b;
}

void writeTile(int num, int addr, byte data) {
  writeEEPROM(tileDevice(num), addr, data);
}

void briefSelectBank()
{
  int num = brief::pop();
  selectBank(num);
}

void briefReadTile()
{
  int addr = brief::pop();
  int num = brief::pop();
  brief::push(readTile(num, addr));
}

void briefWriteTile()
{
  int data = brief::pop();
  int addr = brief::pop();
  int num = brief::pop();
  writeTile(num, addr, data);
}

void setup()
{
  brief::setup(19200);

  Wire.begin();

  pinMode(A_PIN, OUTPUT);
  pinMode(B_PIN, OUTPUT);
  pinMode(C_PIN, OUTPUT);
  pinMode(D_PIN, OUTPUT);
    
  brief::bind(100, briefReadTile);
  brief::bind(101, briefReadTile);
  brief::bind(102, briefReadTile);

  /* Serial.println("Test -----------------------------------");
  for (int i = 0; i < 8; i++) {
    selectBank(i);
    for (int j = 0; j < 4; j++) {
      readTile(j, 0);
    }
  }
  //*/
}

void loop()
{
  brief::loop();
}
