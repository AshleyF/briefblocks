#include <Arduino.h>
#include <Wire.h>
#include <Brief.h>

#define EEPROM_ADDRESS (uint8_t)0x50 // 1010000

#define A_PIN 5
#define B_PIN 4
#define C_PIN 3
#define D_PIN 2

uint8_t readEEPROM(uint8_t device, uint8_t addr) {
  Wire.beginTransmission(device); // e.g. 0x50
  Wire.write(addr);
  Wire.endTransmission();
  Wire.requestFrom(device, (uint8_t)1); // TODO: read multiple
  if (Wire.available()) return Wire.read();
  //Serial.println("Failed to read EEPROM!");
  return 255;
  // TODO: read consecutive without end/begin
}

void writeEEPROM(uint8_t device, uint8_t addr, uint8_t data) {
  Wire.beginTransmission(device); // e.g. 0x50
  Wire.write(addr);
  Wire.write(data);
  Wire.endTransmission();
  delay(10); // have observed failures of consecutive writes to same tile without this
  // TODO: write consecutive bytes without end/begin
}

void selectTile(int16_t num) {
  //Serial.print("Tile ");
  //Serial.println(num);
  digitalWrite(A_PIN, (num & 1) != 0 ? HIGH : LOW);
  digitalWrite(B_PIN, (num & 2) != 0 ? HIGH : LOW);
  digitalWrite(C_PIN, (num & 4) != 0 ? HIGH : LOW);
  digitalWrite(D_PIN, (num & 8) != 0 ? HIGH : LOW);
}

uint8_t readTile(int16_t addr) {
  //Serial.print("  Fetch ");
  //Serial.print(addr);
  //Serial.print(": ");
  uint8_t b = readEEPROM(EEPROM_ADDRESS, addr);
  //Serial.println(b);
  return b;
}

void writeTile(int16_t addr, uint8_t data) {
  //Serial.print("  Store ");
  //Serial.print(addr);
  //Serial.print(" = ");
  //Serial.println(data);
  writeEEPROM(EEPROM_ADDRESS, addr, data);
}

void userError() {
  // TODO
}

// TODO: move this into Brief library

int16_t here = 0;

void memclear() {
  here = 0;
}

void memappend(uint8_t b) {
  if (here > MEM_SIZE) {
    userError(); // exceeded Brief memory
  }
  brief::memset(here++, b);
}

// save all of memory to tile
void saveToTile(int16_t num) {
  if (here > 255) {
    userError(); // exceeded tile memory
    return;
  }

  selectTile(num);
  writeTile(0, here); // length
  for (int16_t i = 0; i < here; i++) {
    writeTile(i + 1, brief::memget(i));
  }
}

// append to memory from single tile
void appendFromTile(int16_t num) {
  selectTile(num);
  int16_t len = readTile(0);
  if (len != 255) {
    for (int16_t i = 0; i < len; i++) {
      memappend(readTile(i + 1));
    }
  }
}

// load (replace) memory from all tiles
void loadFromTiles() {
  // TODO: allow running from tiles one by one (with repeats)
  // TODO: support parameter strip
  memclear();
  for (int16_t i = 14; i >= 0; i--) {
    appendFromTile(i);
  }
  brief::memset(here, 0); // final `return` instruction
}

void dumpMemory() {
  Serial.print("Memory: ");
  for (int16_t i = 0; i <= here; i++) {
    Serial.print(brief::memget(i));
    Serial.print(", ");
  }
  Serial.println();
}

void briefSelectTile() {
  selectTile(brief::pop());
}

void briefReadTile() {
  brief::push(readTile(brief::pop()));
}

void briefWriteTile() {
  int16_t data = brief::pop();
  int16_t addr = brief::pop();
  writeTile(addr, data);
}

void briefForward() {
  Serial.print("FORWARD ");
  Serial.println(brief::pop());
}

void briefLeft() {
  Serial.print("LEFT ");
  Serial.println(brief::pop());
}

void briefRight() {
  Serial.print("RIGHT ");
  Serial.println(brief::pop());
}

void briefRepeat() {
  Serial.print("REPEAT ");
  Serial.println(brief::pop());
}

void testClearTiles() {
  for (int16_t i = 0; i < 16; i++) {
    selectTile(i);
    writeTile(0, 0);
  }  
}

void testStoreSequence() {
  for (int16_t i = 0; i < 16; i++) {
    selectTile(i);
    writeTile(0, 1); // length
    writeTile(1, i);
  }
}

void testMakeTileProgramSet() {
  for (int16_t i = 14; i >= 0; i--) {
    selectTile(i);
    writeTile(0, 1); // length
    writeTile(1, i % 4 == 0 ? 100 : i % 4 == 1 ? 101 : i % 4 == 2 ? 102 : 103);
  }
}

void testMakeTileNumberSet() {
  for (int16_t i = 14; i >= 0; i--) {
    selectTile(i);
    writeTile(0, 2); // length
    writeTile(1, 1); // LIT8
    writeTile(2, 14 - i);
  }
}

void setup() {
  brief::setup();

  Wire.begin();

  pinMode(A_PIN, OUTPUT);
  pinMode(B_PIN, OUTPUT);
  pinMode(C_PIN, OUTPUT);
  pinMode(D_PIN, OUTPUT);
    
  brief::bind(100, briefForward);
  brief::bind(101, briefLeft);
  brief::bind(102, briefRight);
  brief::bind(103, briefRepeat);
  
  brief::bind(200, briefSelectTile);
  brief::bind(201, briefReadTile);
  brief::bind(202, briefWriteTile);

  Serial.println("Test -----------------------------------");
  
  // testClearTiles();
  // testStoreSequence();
  // testMakeTileProgramSet();
  // testMakeTileNumberSet();
  
  loadFromTiles();
  dumpMemory();

  brief::exec(0);
}

void loop() {
  brief::loop();
}
