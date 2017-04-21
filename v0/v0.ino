#include <Arduino.h>
#include <Wire.h>
#include <Brief.h>
#include <AccelStepper.h>

//#define CONSOLE_MODE
#define CNC

#define EEPROM_ADDRESS (uint8_t)0x50 // 1010000

#define A_PIN 5
#define B_PIN 4
#define C_PIN 3
#define D_PIN 2

#define LED_PIN 13

#define RUN_PIN 0
unsigned long lastRunDebounce = 0;
int lastRunState = LOW;
int currentRunState = LOW;

#define SAVE_PIN 1
unsigned long lastSaveDebounce = 0;
int lastSaveState = LOW;
int currentSaveState = LOW;

#define DEBOUNCE_DELAY 50

// #define STEP_TYPE 8 // half-step
#define STEP_TYPE AccelStepper::FULL4WIRE
#define MOTOR_PIN1 6
#define MOTOR_PIN2 7
#define MOTOR_PIN3 8
#define MOTOR_PIN4 9
#define MOTOR_PIN5 10
#define MOTOR_PIN6 11
#define MOTOR_PIN7 12
#define MOTOR_PIN8 13
#define MM 100.0
#define STEPPER_SPEED 600 // steps per second

#define X_PIN A0
#define Y_PIN A1

AccelStepper stepperX(STEP_TYPE, MOTOR_PIN1, MOTOR_PIN3, MOTOR_PIN2, MOTOR_PIN4);
AccelStepper stepperY(STEP_TYPE, MOTOR_PIN5, MOTOR_PIN7, MOTOR_PIN6, MOTOR_PIN8);

#define SIZE   225
#define MIN_X -100
#define MAX_X  100
#define MIN_Y -100
#define MAX_Y  100

void moveCNC(double dx, double dy) {
  Serial.print("    Relative Move: ");
  Serial.print(dx);
  Serial.print(" ");
  Serial.println(dy);
#ifdef CNC
  long distanceX = dx * MM;
  long distanceY = dy * MM;
  long stepperSpeedX = STEPPER_SPEED;
  long stepperSpeedY = STEPPER_SPEED;
  if (abs(distanceX) >= abs(distanceY)) {
    stepperSpeedY = long(STEPPER_SPEED) * long(abs(distanceY)) / long(abs(distanceX));
  } else {
    stepperSpeedX = long(STEPPER_SPEED) * long(abs(distanceX)) / long(abs(distanceY));
  }
  if (stepperSpeedX == 0) stepperSpeedX = 1;
  if (stepperSpeedY == 0) stepperSpeedY = 1;
  Serial.print("Stepper speeds: ");
  Serial.print(stepperSpeedX);
  Serial.println(stepperSpeedY);
  stepperX.move(distanceX);
  stepperY.move(distanceY);
  long stepsX = stepperX.distanceToGo();
  long stepsY = stepperY.distanceToGo();
  while (stepsX != 0 || stepsY != 0) {
    stepperX.setSpeed(stepperSpeedX);
    stepperY.setSpeed(stepperSpeedY);
    stepperX.runSpeedToPosition();
    stepperY.runSpeedToPosition();
    stepsX = stepperX.distanceToGo();
    stepsY = stepperY.distanceToGo();
  }
#endif
}

double theta = 0.0; // degrees
double x = 0.0;
double y = 0.0;

double lastX = 0.0;
double lastY = 0.0;

void update() {
  Serial.print("    Pose: ");
  Serial.print((int)theta);
  Serial.print(" ");
  Serial.print((int)(x + 0.5));
  Serial.print(" ");
  Serial.println((int)(y + 0.5));
  // enforce physical bounds
  double xx = min(MAX_X, max(MIN_X, x));
  double yy = min(MAX_Y, max(MIN_Y, y));
  double dx = xx - lastX;
  double dy = yy - lastY;
  lastX = xx;
  lastY = yy;
  moveCNC(dx, dy);
}

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

void blinkLed() {
  const int16_t pause = 300;
  digitalWrite(LED_PIN, HIGH);
  delay(pause);
  digitalWrite(LED_PIN, LOW);
  delay(pause);
}

void userError() {
  for (int16_t i = 0; i < 5; i++) {
    blinkLed();
  }
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

void runPressed() {
  blinkLed();
  loadFromTiles();
  dumpMemory();
  brief::exec(0);
  blinkLed();
}

void savePressed() {
  blinkLed();
  loadFromTiles();
  dumpMemory();
  saveToTile(15);
  blinkLed();
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

void briefGoXY() {
  Serial.print("GO ");
  y = brief::pop();
  x = brief::pop();
  Serial.print(x);
  Serial.print(",");
  Serial.println(y);
  update();
}

void briefHead() {
  Serial.print("HEAD ");
  theta = brief::pop();
  Serial.println(theta);
  update();
}

void briefForward() {
  Serial.print("FORWARD ");
  double d = brief::pop();
  Serial.println(d);
  x += d * cos(theta / 180.0 * PI);
  y += d * sin(theta / 180.0 * PI);
  update(); 
}

void briefLeft() {
  Serial.print("LEFT ");
  double a = brief::pop();
  Serial.println(a);
  theta += -a;
  update();
}

void briefRight() {
  Serial.print("RIGHT ");
  double a = brief::pop();
  Serial.println(a);
  theta += -a;
  update();
}

void briefRepeat() {
  Serial.print("REPEAT ");
  // TODO
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

void joystick(int x, int y) {
  int dx = x > 700 ? 1 : x < 300 ? -1 : 0;
  int dy = y > 700 ? -1 : y < 300 ? 1 : 0;
  if (dx != 0 || dy != 0) {
    moveCNC(dx, dy);
    theta = x = y = lastX = lastY = 0.0;
  }
}

void setup() {
  brief::setup();

  Wire.begin();

  pinMode(A_PIN, OUTPUT);
  pinMode(B_PIN, OUTPUT);
  pinMode(C_PIN, OUTPUT);
  pinMode(D_PIN, OUTPUT);

  pinMode(LED_PIN, OUTPUT);

  pinMode(RUN_PIN, INPUT);
  pinMode(SAVE_PIN, INPUT);

  pinMode(X_PIN, INPUT);
  pinMode(Y_PIN, INPUT);

  stepperX.setMaxSpeed(STEPPER_SPEED);
  stepperY.setMaxSpeed(STEPPER_SPEED);
    
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
  saveToTile(15);

  brief::exec(0);
}

void loop() {
  brief::loop();

  joystick(analogRead(X_PIN), analogRead(Y_PIN));

  int16_t runState = digitalRead(RUN_PIN);
  if (runState != lastRunState) lastRunDebounce = millis();
  if ((millis() - lastRunDebounce) > DEBOUNCE_DELAY) {
    if (runState != currentRunState) {
      currentRunState = runState;
      if (runState == HIGH) runPressed();
    }
  }
  lastRunState = runState;

  int16_t saveState = digitalRead(SAVE_PIN);
  if (saveState != lastSaveState) lastSaveDebounce = millis();
  if ((millis() - lastSaveDebounce) > DEBOUNCE_DELAY) {
    if (saveState != currentSaveState) {
      currentSaveState = saveState;
      if (saveState == HIGH) savePressed();
    }
  }
  lastSaveState = saveState;
}
