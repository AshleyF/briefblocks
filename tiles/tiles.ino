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
  // delay(10);
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

unsigned int tileAddress(int num) {
  switch (num) {
    case 0: return EEPROM_ADDRESS_0;
    case 1: return EEPROM_ADDRESS_1;
    case 2: return EEPROM_ADDRESS_2;
    case 3: return EEPROM_ADDRESS_3;
    default: return 0; // TODO: error if not 0 >= num <= 3
  }
}

void writeTile(int num, int addr, byte data) {
  writeEEPROM(tileAddress(num), addr, data);
}

void readTile(int num, int addr) {
  Serial.print("  Tile ");
  Serial.print(num);
  Serial.print(": ");
  byte b = readEEPROM(tileAddress(num), addr);
  Serial.println(b);
}

void selectBank(int num) {
  Serial.print("Bank ");
  Serial.println(num);
  digitalWrite(A_PIN, (num & 1) != 0 ? HIGH : LOW);
  digitalWrite(B_PIN, (num & 2) != 0 ? HIGH : LOW);
  digitalWrite(C_PIN, (num & 4) != 0 ? HIGH : LOW);
  delay(10);
}

void testReadTiles(int num) {
  selectBank(num);
  for (int i = 0; i < 4; i++) {
    readTile(i, 0);
  }
}

void setup() {
  Serial.begin(9600);
  Wire.begin();
  pinMode(A_PIN, OUTPUT);
  pinMode(B_PIN, OUTPUT);
  pinMode(C_PIN, OUTPUT);

  //writeTile(1, 1, 1);

  Serial.println("Test -----------------------------------");

  for (int i = 0; i < 8; i++) {
    testReadTiles(i);
  }
}

void loop() {
}
