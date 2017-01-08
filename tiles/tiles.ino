#include <Wire.h>

#define EEPROM_ADDRESS_0 (byte)0x50 // 1010000
#define EEPROM_ADDRESS_1 (byte)0x51 // 1010001
#define EEPROM_ADDRESS_2 (byte)0x52 // 1010010
#define EEPROM_ADDRESS_3 (byte)0x53 // 1010011

#define A_PIN 2
#define B_PIN 3
#define C_PIN 4

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
  //delay(100);
  Wire.requestFrom(device, (byte)1);
  if (Wire.available()) return Wire.read();
  return 0;
}

void readTile(unsigned int address) {
  byte b = readEEPROM(address, 0);
  Serial.println(b);
}

void setup() {
  Serial.begin(9600);
  Wire.begin();
  pinMode(A_PIN, OUTPUT);
  pinMode(B_PIN, OUTPUT);
  pinMode(C_PIN, OUTPUT);

  digitalWrite(A_PIN, LOW);
  digitalWrite(B_PIN, LOW);
  digitalWrite(C_PIN, LOW);
  delay(100);

  //writeEEPROM(EEPROM_ADDRESS_0, 0, 2);
  //writeEEPROM(EEPROM_ADDRESS_1, 0, 2);

  Serial.println("Test -----------------------------------");
  Serial.print("Bank 0, Tile 0 -> ");
  readTile(EEPROM_ADDRESS_0);
  Serial.print("Bank 0, Tile 1 -> ");
  readTile(EEPROM_ADDRESS_1);
  Serial.print("Bank 0, Tile 2 -> ");
  readTile(EEPROM_ADDRESS_2);
  Serial.print("Bank 0, Tile 3 -> ");
  readTile(EEPROM_ADDRESS_3);

  digitalWrite(A_PIN, HIGH);
  digitalWrite(B_PIN, LOW);
  digitalWrite(C_PIN, LOW);
  delay(100);

  Serial.print("Bank 1, Tile 0 -> ");
  readTile(EEPROM_ADDRESS_0);
  Serial.print("Bank 1, Tile 1 -> ");
  readTile(EEPROM_ADDRESS_1);
  Serial.print("Bank 1, Tile 2 -> ");
  readTile(EEPROM_ADDRESS_2);
  Serial.print("Bank 1, Tile 3 -> ");
  readTile(EEPROM_ADDRESS_3);

  digitalWrite(A_PIN, LOW);
  digitalWrite(B_PIN, HIGH);
  digitalWrite(C_PIN, LOW);
  delay(100);

  Serial.print("Bank 2, Tile 0 -> ");
  readTile(EEPROM_ADDRESS_0);
  Serial.print("Bank 2, Tile 1 -> ");
  readTile(EEPROM_ADDRESS_1);
  Serial.print("Bank 2, Tile 2 -> ");
  readTile(EEPROM_ADDRESS_2);
  Serial.print("Bank 2, Tile 3 -> ");
  readTile(EEPROM_ADDRESS_3);

  digitalWrite(A_PIN, HIGH);
  digitalWrite(B_PIN, HIGH);
  digitalWrite(C_PIN, LOW);
  delay(100);

  Serial.print("Bank 3, Tile 0 -> ");
  readTile(EEPROM_ADDRESS_0);
  Serial.print("Bank 3, Tile 1 -> ");
  readTile(EEPROM_ADDRESS_1);
  Serial.print("Bank 3, Tile 2 -> ");
  readTile(EEPROM_ADDRESS_2);
  Serial.print("Bank 3, Tile 3 -> ");
  readTile(EEPROM_ADDRESS_3);
}

void loop() {
}
