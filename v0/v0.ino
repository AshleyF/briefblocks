#include <Arduino.h>
#include <Wire.h>
//#include <Brief.h>

#define EEPROM_ADDRESS (byte)0x50 // 1010000

#define A_PIN 5
#define B_PIN 4
#define C_PIN 3
#define D_PIN 2

byte readEEPROM(byte device, byte address) {
  Wire.beginTransmission(device); // e.g. 0x50
  Wire.write(address);
  Wire.endTransmission();
  Wire.requestFrom(device, 1); // TODO: read multiple
  if (Wire.available()) return Wire.read();
  Serial.println("Failed to read EEPROM!");
  return 255;
}

void writeEEPROM(byte device, byte address, byte data) {
  Wire.beginTransmission(device); // e.g. 0x50
  Wire.write(address);
  Wire.write(data);
  Wire.endTransmission();
}

void selectTile(int num) {
  Serial.print("Tile ");
  Serial.println(num);
  digitalWrite(A_PIN, (num & 1) != 0 ? HIGH : LOW);
  digitalWrite(B_PIN, (num & 2) != 0 ? HIGH : LOW);
  digitalWrite(C_PIN, (num & 4) != 0 ? HIGH : LOW);
  digitalWrite(D_PIN, (num & 8) != 0 ? HIGH : LOW);
}

byte readTile(int addr) {
  Serial.print("  Value ");
  Serial.print(addr);
  Serial.print(": ");
  byte b = readEEPROM(EEPROM_ADDRESS, addr);
  Serial.println(b);
  return b;
}

void writeTile(int addr, byte data) {
  writeEEPROM(EEPROM_ADDRESS, addr, data);
}

/*
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
//*/

void setup()
{
  Serial.begin(9600);
  //brief::setup(19200);

  Wire.begin();

  pinMode(A_PIN, OUTPUT);
  pinMode(B_PIN, OUTPUT);
  pinMode(C_PIN, OUTPUT);
  pinMode(D_PIN, OUTPUT);
    
  //brief::bind(100, briefReadTile);
  //brief::bind(101, briefReadTile);
  //brief::bind(102, briefReadTile);

  // Serial.println("Test -----------------------------------");
  /*
  for (int i = 0; i < 16; i++) {
    selectTile(i);
    writeTile(0, 42 + i);
  }
  //*/
  for (int i = 0; i < 16; i++) {
    selectTile(i);
    readTile(0);
  }
  //*/
}

void loop()
{
  //brief::loop();
}
