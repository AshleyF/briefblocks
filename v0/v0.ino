#include <Arduino.h>
#include <Wire.h>
#include <Brief.h>
#include <AccelStepper.h>

#define RETURN_INSTRUCTION 0
#define LIT8_INSTRUCTION 1
#define LIT16_INSTRUCTION 2
#define PUSHR_INSTRUCTION 38
#define NEXT_INSTRUCTION 58

//#define CONSOLE_MODE
#define CNC

#define EEPROM_ADDRESS (byte)0x50 // 1010000

#define A_PIN 5
#define B_PIN 4
#define C_PIN 3
#define D_PIN 2

#define LED_PIN 13

#define RUN_PIN A2
unsigned long lastRunDebounce = 0;
bool lastRunState = false;
bool currentRunState = false;

#define DEBOUNCE_DELAY 50

#define STEP_TYPE 8 // half-step
//#define STEP_TYPE AccelStepper::FULL4WIRE
#define DRIVE_FACTOR 3
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

#define X_PIN A1
#define Y_PIN A0

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

byte readEEPROM(byte device, byte addr) {
  Wire.beginTransmission(device); // e.g. 0x50
  Wire.write(addr);
  Wire.endTransmission();
  Wire.requestFrom(device, (byte)1); // TODO: read multiple
  if (Wire.available()) return Wire.read();
  //Serial.println("Failed to read EEPROM!");
  return 255;
  // TODO: read consecutive without end/begin
}

void writeEEPROM(byte device, byte addr, byte data) {
  Serial.print("WRITE ");
  Serial.print(addr);
  Serial.print(" ");
  Serial.println(data);
  Wire.beginTransmission(device); // e.g. 0x50
  Wire.write(addr);
  Wire.write(data);
  Wire.endTransmission();
  // delay(10); // have observed failures of consecutive writes to same tile without this
  // TODO: write consecutive bytes without end/begin
}

void selectTile(int num) {
  //Serial.print("Tile ");
  //Serial.println(num);
  digitalWrite(A_PIN, (num & 1) != 0 ? HIGH : LOW);
  digitalWrite(B_PIN, (num & 2) != 0 ? HIGH : LOW);
  digitalWrite(C_PIN, (num & 4) != 0 ? HIGH : LOW);
  digitalWrite(D_PIN, (num & 8) != 0 ? HIGH : LOW);
}

byte readTile(int addr) {
  //Serial.print("  Fetch ");
  //Serial.print(addr);
  //Serial.print(": ");
  byte b = readEEPROM(EEPROM_ADDRESS, addr);
  //Serial.println(b);
  return b;
}

void writeTile(int addr, byte data) {
  Serial.print("  Store ");
  Serial.print(addr);
  Serial.print(" = ");
  Serial.println(data);
  writeEEPROM(EEPROM_ADDRESS, addr, data);
}

void blinkLed() {
  /*
  const int pause = 300;
  digitalWrite(LED_PIN, HIGH);
  delay(pause);
  digitalWrite(LED_PIN, LOW);
  delay(pause);
  //*/
}

void userError() {
  for (int i = 0; i < 5; i++) {
    blinkLed();
  }
}

// TODO: move this into Brief library

int here = 0;

void memclear() {
  here = 0;
}

void memappend(byte b) {
  if (here >= MEM_SIZE) {
    userError(); // exceeded Brief memory
  }
  brief::memset(here++, b);
}

// save all of memory to tile
void saveToTile(int num) {
  if (here > 255) {
    userError(); // exceeded tile memory
    return;
  }

  selectTile(num);
  Wire.beginTransmission(EEPROM_ADDRESS);
  Wire.write(0);
  Wire.write(here);
  //writeTile(0, here); // length
  for (int i = 0; i < here; i++) {
    Wire.write(brief::memget(i));
    //writeTile(i + 1, brief::memget(i));
  }
  Wire.endTransmission();
}

// append to memory from single tile
void appendFromTile(int num) {
  selectTile(num);
  int len = readTile(0);
  if (len != 255) {
    for (int i = 0; i < len; i++) {
      byte b = readTile(i + 1);
      if (b == 255) { // special `repeat` macro
        // example `x y z lit8 10` becomes `lit8 10 x y z next 3`
        // determine length of literal
        int len = 0;
        if (brief::memget(here - 3) == LIT16_INSTRUCTION) { // `lit16`
          len = 3;
        } else if (brief::memget(here - 2) == LIT8_INSTRUCTION) { // `lit8`
          len = 2;
        } else {
          userError(); // `repeat` must be preceeded by literal
          return;
        }
        // shift everything over by `len`
        here += len;
        if (here >= MEM_SIZE) {
          userError(); // expanded macro will exceed memory
          return;
        }
        for (int j = here; j > len; j--) {
          brief::memset(j, brief::memget(j - len - 1));
        }
        // move literal to front
        here -= (len - 1); // literal addr
        for (int j = 0; j < len; j++) {
          brief::memset(j, brief::memget(here + j));
        }
        brief::memset(len, PUSHR_INSTRUCTION);
        // append `next`
        memappend(NEXT_INSTRUCTION);
        memappend(here - len - 2); // length of loop body
      } else {
        memappend(b);
      }
    }
  }
}

// load (replace) memory from all tiles
void loadFromTiles() {
  // TODO: allow running from tiles one by one (with repeats)
  // TODO: support parameter strip
  memclear();
  for (int i = 14; i >= 0; i--) {
    appendFromTile(i);
  }
  brief::memset(here, RETURN_INSTRUCTION);
}

void saveAndRun() {
  Serial.println("SAVE AND RUN");
  //blinkLed();
  loadFromTiles();
  dumpMemory();
  saveToTile(15);
  brief::exec(0);
  //blinkLed();
}

void dumpMemory() {
  Serial.print("Memory: ");
  for (int i = 0; i <= here; i++) {
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
  int data = brief::pop();
  int addr = brief::pop();
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
  double d = brief::pop() * DRIVE_FACTOR;
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

void testClearTiles() {
  for (int i = 0; i < 16; i++) {
    selectTile(i);
    writeTile(0, 0);
  }  
}

void testStoreSequence() {
  for (int i = 0; i < 16; i++) {
    selectTile(i);
    writeTile(0, 1); // length
    writeTile(1, i);
  }
}

void testMakeTileProgramSet() {
  for (int i = 14; i >= 0; i--) {
    selectTile(i);
    writeTile(0, 1); // length
    writeTile(1, i % 4 == 0 ? 100 : i % 4 == 1 ? 101 : i % 4 == 2 ? 102 : 103);
  }
}

void testMakeTileNumberSet() {
  for (int i = 14; i >= 0; i--) {
    selectTile(i);
    writeTile(0, 2); // length
    writeTile(1, LIT8_INSTRUCTION);
    writeTile(2, 14 - i);
  }
}

void joystick(int x, int y) {
  int dx = x > 700 ? -1 : x < 300 ? 1 : 0;
  int dy = y > 700 ? 1 : y < 300 ? -1 : 0;
  if (dx != 0 || dy != 0) {
    moveCNC(dy, dx);
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

  pinMode(X_PIN, INPUT);
  pinMode(Y_PIN, INPUT);

  stepperX.setMaxSpeed(STEPPER_SPEED);
  stepperY.setMaxSpeed(STEPPER_SPEED);
    
  brief::bind(100, briefForward);
  brief::bind(101, briefLeft);
  brief::bind(102, briefRight);
  
  brief::bind(200, briefSelectTile);
  brief::bind(201, briefReadTile);
  brief::bind(202, briefWriteTile);

  // Serial.println("Test -----------------------------------");
  
  // testClearTiles();
  // testStoreSequence();
  // testMakeTileProgramSet();
  // testMakeTileNumberSet();
  
  // loadFromTiles();
  // dumpMemory();
  // saveToTile(15);

  // brief::exec(0);

  /* save literal
  selectTile(14);
  writeTile(0, 2); // length
  writeTile(1, LIT8_INSTRUCTION);
  writeTile(2, 90);
  //*/

  /* save instructionr
  selectTile(14);
  writeTile(0, 1); // length
  writeTile(1, 100); // forward
  selectTile(13);
  writeTile(0, 1); // length
  writeTile(1, 101); // left
  selectTile(12);
  writeTile(0, 1); // length
  writeTile(1, 102); / right
  selectTile(11);
  writeTile(0, 1); // length
  writeTile(1, 255); // repeat
  //*/
}

void loop() {
  brief::loop();

  joystick(analogRead(X_PIN), analogRead(Y_PIN));

  bool runState = (analogRead(RUN_PIN) == 0);
  if (runState != lastRunState) lastRunDebounce = millis();
  if ((millis() - lastRunDebounce) > DEBOUNCE_DELAY) {
    if (runState != currentRunState) {
      currentRunState = runState;
      if (runState) {
        saveAndRun();
      }
    }
  }
  lastRunState = runState;
}
