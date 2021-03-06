#include <Arduino.h>
#include <Wire.h>
#include <AccelStepper.h>

//#define CONSOLE_MODE
#define CNC

// #define STEP_TYPE 8 // half-step
#define STEP_TYPE AccelStepper::FULL4WIRE
#define MOTOR_PIN1 5
#define MOTOR_PIN2 6
#define MOTOR_PIN3 7
#define MOTOR_PIN4 8
#define MOTOR_PIN5 9
#define MOTOR_PIN6 10
#define MOTOR_PIN7 11
#define MOTOR_PIN8 12
#define X_PIN A0
#define Y_PIN A1
#define MM 100.0
#define STEPPER_SPEED 600 // steps per second

AccelStepper stepperX(STEP_TYPE, MOTOR_PIN1, MOTOR_PIN3, MOTOR_PIN2, MOTOR_PIN4);
AccelStepper stepperY(STEP_TYPE, MOTOR_PIN5, MOTOR_PIN7, MOTOR_PIN6, MOTOR_PIN8);

#define EEPROM_ADDRESS_0 (byte)0x50 // 1010000
#define EEPROM_ADDRESS_1 (byte)0x51 // 1010001

#define RESET_PIN 0
#define PULSE_PIN 1
#define BUTTON 13

#define SIZE   225
#define MIN_X -100
#define MAX_X  100
#define MIN_Y -100
#define MAX_Y  100

#define MEM_SIZE         512
#define MAX_DATA_STACK   8 // circular stack (see below)
#define MAX_RETURN_STACK 8
#define MAX_BLOCKS       256
#define MAX_PRIMITIVES   32

#define RET     0
#define LIT     1
#define DIG0    2
#define DIG1    3
#define DIG2    4
#define DIG3    5
#define DIG4    6
#define DIG5    7
#define DIG6    8
#define DIG7    9
#define DIG8    10
#define DIG9    11
#define MOD     12
#define MUL     13
#define ADD     14
#define SUB     15
#define SHOW    16
#define DIV     17
#define BACK    18
#define CENTER  19
#define FORWARD 20
#define GO      21
#define HEAD    22
#define LEFT    23
#define RIGHT   24
#define TURN    25
#define TIMES   26
#define RESET   27
#define DUP     28
#define NOP     31

#define DEF_BLOCK 0x5B // define block ID (':' == 58)
#define TIMES_BLOCK0 0x19

uint8_t memory[MEM_SIZE];
uint8_t mem(int16_t address) { return memory[address]; } // TODO: bounds check
void memset(int16_t address, uint8_t value) { memory[address] = value; } // TODO: bounds check

int16_t dstack[MAX_DATA_STACK];
int16_t* s = dstack - 1;
void push(int16_t i) {
  if (s >= dstack + MAX_DATA_STACK - 1) s = dstack - 1; // wrap
  *(++s) = i;
}
int16_t pop() {
  return *s--;
  if (s < dstack - 1) s = dstack + MAX_DATA_STACK - 1; // wrap
}

int16_t rstack[MAX_RETURN_STACK];
int16_t* r = rstack - 1;
void rpush(int16_t i) { *(++r) = i; } // TODO: overflow check
int16_t rpop() { return *r--; } // TODO: underflow check

void (*instructions[MAX_PRIMITIVES])(); // instruction function table
void bind(uint8_t i, void (*f)()) { instructions[i] = f; }

int16_t blocks[MAX_BLOCKS]; // block call table

int16_t p; // program counter (VM instruction pointer)

void ret() { Serial.println("    RETURN"); p = rpop(); }

void nop() { Serial.println("NOP"); }

void lit() { Serial.println("LIT"); push(0); }
void zero()  { Serial.println("0"); push(pop() * 10 + 0); }
void one()   { Serial.println("1"); push(pop() * 10 + 1); }
void two()   { Serial.println("2"); push(pop() * 10 + 2); }
void three() { Serial.println("3"); push(pop() * 10 + 3); }
void four()  { Serial.println("4"); push(pop() * 10 + 4); }
void five()  { Serial.println("5"); push(pop() * 10 + 5); }
void six()   { Serial.println("6"); push(pop() * 10 + 6); }
void seven() { Serial.println("7"); push(pop() * 10 + 7); }
void eight() { Serial.println("8"); push(pop() * 10 + 8); }
void nine()  { Serial.println("9"); push(pop() * 10 + 9); }

void show() { Serial.print("SHOW: "); Serial.println(pop(), DEC); }

void add() { Serial.println("ADD"); push(pop() + pop()); dup(); show(); } // TODO: remove dup/show
void sub() { Serial.println("SUB"); push(pop() - pop()); }
void mul() { Serial.println("MUL"); push(pop() * pop()); }
void div() { Serial.println("DIV"); push(pop() / pop()); }
void mod() { Serial.println("MOD"); push(pop() % pop()); }
void neg() { Serial.println("NEG"); push(-pop()); }

double theta = 0.0; // degrees
double x = 0.0;
double y = 0.0;

double lastX = 0.0;
double lastY = 0.0;

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

void resetCNC() {
  Serial.println("RESET CNC");
  moveCNC(SIZE, SIZE);
  moveCNC(-MAX_X, -MAX_Y);
  theta = x = y = lastX = lastY = 0.0;
}

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

void go() { Serial.println("GO"); y = pop(); x = pop(); update(); }
void head() { Serial.println("HEAD"); theta = pop(); update();}
void center() { Serial.println(":CENTER"); lit(); lit(); go(); lit(); head(); } // TODO: secondary word?
void turn() { Serial.println("TURN"); theta += pop(); update(); }
void left() { Serial.println(":LEFT"); neg(); turn(); } // TODO: secondary word?
void right() { Serial.println(":RIGHT"); turn(); } // TODO: secondary word?
void forward() { Serial.println("FORWARD"); double d = pop(); x += d * cos(theta / 180.0 * PI); y += d * sin(theta / 180.0 * PI); update(); }
void back() { Serial.println(":BACK"); neg(); forward(); } // TODO: secondary word?
void times() {
  Serial.print("TIMES INSTRUCTION: ");
  Serial.print("P: ");
  Serial.println(p);
  int16_t pp = p - 1;
  while (pp >= 0 && mem(pp--) != LIT); // assumes previous LIT
  pp--; // to CALL
  Serial.print("PP: ");
  Serial.println(pp);
  if (pp > 0) {
    int16_t save = mem(pp);
    memset(pp, RET); // temporarily replace with RET
    int16_t addr = pop();
    int16_t x = pop();
    for (int i = 0; i < x - 1; i++) { // -1 because it's done once before getting to TIMES block
      Serial.print("REPEAT: ");
      Serial.println(i);
      call(addr);
    }
    memset(pp, save); // replace call
  }
}
void dup() { Serial.println("DUP"); int16_t x = pop(); push(x); push(x); }

void run() {
  int16_t i;
  do {
    i = mem(p++);
    Serial.print("    Instruction: ");
    Serial.println(i, DEC);
    if ((i & 0x40) == 0) { // instruction? (7th bit set is a call)
      instructions[i](); // execute
    } else { // address to call
      if (mem(p + 1) != 0) rpush(p + 1); // push return address if not followed by return (TCO)
      p = ((i << 8) & 0x3f00) | mem(p); // jump (address is lower 6 bits)
    }
  } while (p >= 0); // note: -1 initially pushed to return stack
}

void exec(int16_t address) {
  Serial.print("EXEC: ");
  Serial.println(address);
  r = rstack - 1; // reset return stack
  s = dstack - 1; // reset data stack
  p = address;
  rpush(-1); // causing run() to fall through upon completion
  run();
}

void call(int16_t address) {
  Serial.print("CALL: ");
  Serial.println(address);
  int16_t pp = p;
  rpush(-1); // causing run() to fall through upon completion
  p = address;
  run();
  p = pp;
}

uint16_t dict = 0;
uint16_t here = 0;
void append(uint8_t b) {
    Serial.print("    Append: ");
    Serial.println(b, DEC);
    memset(here++, b);
}

uint16_t define() {
  uint16_t w = dict;
  dict = here;
  Serial.print("    Define - address: ");
  Serial.println(dict, DEC);
  return w;
}

uint16_t defineOp(uint8_t op) {
  append(op);
  append(RET);
  return define();
}

void reset() {
  Serial.println("    Reset");
  digitalWrite(RESET_PIN, LOW);
  delay(10);
  digitalWrite(RESET_PIN, HIGH);
}

void pulse() {
  Serial.println("    Pulse");
  digitalWrite(PULSE_PIN, LOW);
  delay(10);
  digitalWrite(PULSE_PIN, HIGH);  
  delay(290);
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

bool param = true;
uint8_t readNextBlock() {
#ifdef CONSOLE_MODE
  Serial.println("    NEXT CONSOLE BLOCK");
  delay(50); // enforce reading in batch
  if (Serial.available() > 0) return Serial.read();
  Serial.println("    END OF BLOCKS");
  return 0;
#else
  Serial.println("    NEXT PHYSICAL BLOCK");
  if (param) {
    param = false;
    int p = readEEPROM(EEPROM_ADDRESS_1, 0);
    Serial.print("    Chip 1 Parameter: ");
    Serial.println(p, HEX);
    if (p) return p; else return readNextBlock();
  } else {
    param = true;
    Serial.print("    Chip 0 Main: ");
    int b = readEEPROM(EEPROM_ADDRESS_0, 0);
    Serial.println(b, HEX);
    if (b) pulse(); else Serial.println("    END OF BLOCKS");
    return b;
  }
#endif
}

bool readBlocks() {
  reset();
  here = dict;
  uint8_t b;
  do {
    uint8_t b = readNextBlock();
    if (b == 0) break;
    Serial.print("    Read: ");
    Serial.println(b, HEX);
    if (b == DEF_BLOCK) {
      // define new word
      uint8_t d = readNextBlock();
      Serial.print("DEFINE: ");
      Serial.println(d, HEX);
      append(RET);
      blocks[d] = define();
      return;
    } else if (b == TIMES_BLOCK0) {
      Serial.print("TIMES BLOCK: ");
      Serial.println(dict);
      append(LIT);
      int16_t h = dict / 100;
      append(h + DIG0); // TODO: assumes three decimal digits max
      append((dict - h * 100) / 10 + DIG0);
      append(dict % 10 + DIG0);
      append(TIMES);
      continue;
    }
    int16_t addr = blocks[b];
    Serial.print("    Call: ");
    Serial.println(addr, DEC);
    append(addr >> 8 | 0x40);
    append(addr & 0xff);
  } while(here < MEM_SIZE); // TODO: error when exceeding MAX_INSTRUCTIONS?
  reset();
  if (here - dict > 0) {
    append(RET);
    Serial.print("    Execute blocks - address: ");
    Serial.println(dict, DEC);
    exec(dict);
    return true;
  }
  return false;
}

void setup() {
  pinMode(X_PIN, INPUT);
  pinMode(Y_PIN, INPUT);
  pinMode(BUTTON, INPUT);
  pinMode(RESET_PIN, OUTPUT);
  pinMode(PULSE_PIN, OUTPUT);
  Serial.begin(9600);
  Wire.begin();
  stepperX.setMaxSpeed(STEPPER_SPEED);
  stepperY.setMaxSpeed(STEPPER_SPEED);
  pulse();
  // initialize primitive instructions
  for (int i = 0; i < MAX_PRIMITIVES; i++) bind(i, nop);
  bind(RET,     ret);
  bind(LIT,     lit);
  bind(DIG0,    zero);
  bind(DIG1,    one);
  bind(DIG2,    two);
  bind(DIG3,    three);
  bind(DIG4,    four);
  bind(DIG5,    five);
  bind(DIG6,    six);
  bind(DIG7,    seven);
  bind(DIG8,    eight);
  bind(DIG9,    nine);
  bind(MOD,     mod);
  bind(MUL,     mul);
  bind(ADD,     add);
  bind(SUB,     sub);
  bind(SHOW,    show);
  bind(DIV,     div);
  bind(BACK,    back);
  bind(CENTER,  center);
  bind(FORWARD, forward);
  bind(GO,      go);
  bind(HEAD,    head);
  bind(LEFT,    left);
  bind(RIGHT,   right);
  bind(TURN,    turn);
  bind(TIMES,   times);
  bind(RESET,   resetCNC);
  bind(DUP,     dup);
  bind(NOP,     nop);
  // initialize block call table
  uint16_t catchAll = defineOp(NOP);
  for (int i = 0; i < MAX_BLOCKS; i++) blocks[i] = catchAll;
#ifdef CONSOLE_MODE
  blocks[0]   = catchAll;          // nop
  blocks[35]  = defineOp(LIT);     // #
  blocks[37]  = defineOp(MOD);     // %
  blocks[42]  = defineOp(MUL);     // *
  blocks[43]  = defineOp(ADD);     // +
  blocks[45]  = defineOp(SUB);     // -
  blocks[46]  = defineOp(SHOW);    // .
  blocks[47]  = defineOp(DIV);     // /
  blocks[48]  = defineOp(DIG0);    // 0
  blocks[49]  = defineOp(DIG1);    // 1
  blocks[50]  = defineOp(DIG2);    // 2
  blocks[51]  = defineOp(DIG3);    // 3
  blocks[52]  = defineOp(DIG4);    // 4
  blocks[53]  = defineOp(DIG5);    // 5
  blocks[54]  = defineOp(DIG6);    // 6
  blocks[55]  = defineOp(DIG7);    // 7
  blocks[56]  = defineOp(DIG8);    // 8
  blocks[57]  = defineOp(DIG9);    // 9
  blocks[98]  = defineOp(BACK);    // b
  blocks[99]  = defineOp(CENTER);  // c
  blocks[102] = defineOp(FORWARD); // f
  blocks[103] = defineOp(GO);      // g
  blocks[104] = defineOp(HEAD);    // h
  blocks[108] = defineOp(LEFT);    // l
  blocks[114] = defineOp(RIGHT);   // r
  blocks[116] = defineOp(TURN);    // t
  delay(2000);
  Serial.println("INITIALIZED (CONSOLE MODE)");
#else
  // physical block definitions
  blocks[0x1b] = blocks[0x0f] = blocks[0x05] = defineOp(FORWARD); // Forward
  blocks[0x25] = blocks[0x1f] = defineOp(BACK); // Back
  blocks[0x0d] = blocks[0x01] = blocks[0x13] = defineOp(LEFT); // Left
  blocks[0x09] = blocks[0x0b] = defineOp(RIGHT); // Right
  blocks[0x3e] = defineOp(CENTER); // Center
  blocks[0x52] = defineOp(RESET); // Reset
  blocks[0x40] = defineOp(DUP); // Dup
  blocks[0x2f] = defineOp(MOD); // Mod
  blocks[0x2d] = defineOp(MUL); // Mul
  blocks[0x2b] = defineOp(DIV); // Div
  blocks[0x29] = defineOp(ADD); // Add
  blocks[0x21] = defineOp(SUB); // Sub
  append(LIT); append(DIG1); append(RET); blocks[0x1c] = define(); // 1
  append(LIT); append(DIG2); append(RET); blocks[0x2c] = define(); // 2
  append(LIT); append(DIG3); append(RET); blocks[0x0c] = define(); // 3
  append(LIT); append(DIG4); append(RET); blocks[0x36] = define(); // 4
  append(LIT); append(DIG5); append(RET); blocks[0x22] = define(); // 5
  append(LIT); append(DIG6); append(RET); blocks[0x18] = define(); // 6
  append(LIT); append(DIG7); append(RET); blocks[0x24] = define(); // 7
  append(LIT); append(DIG8); append(RET); blocks[0x5f] = define(); // 8
  append(LIT); append(DIG9); append(RET); blocks[0x62] = define(); // 9
  append(LIT); append(DIG1); append(DIG0); append(RET); blocks[0x38] = blocks[0x6e] = blocks[0x63] = define(); // 10
  append(LIT); append(DIG1); append(DIG2); append(RET); blocks[0x28] = blocks[0x12] = define(); // 12
  append(LIT); append(DIG1); append(DIG5); append(RET); blocks[0x16] = blocks[0x14] = define(); // 15
  append(LIT); append(DIG1); append(DIG8); append(RET); blocks[0x2e] = blocks[0x1e] = define(); // 18
  append(LIT); append(DIG2); append(DIG0); append(RET); blocks[0x6b] = blocks[0x67] = define(); // 20
  append(LIT); append(DIG2); append(DIG4); append(RET); blocks[0x0a] = blocks[0x02] = define(); // 24
  append(LIT); append(DIG3); append(DIG0); append(RET); blocks[0x65] = blocks[0x5d] = blocks[0x66] = define(); // 30
  append(LIT); append(DIG3); append(DIG6); append(RET); blocks[0x5e] = blocks[0x26] = define(); // 36
  append(LIT); append(DIG4); append(DIG0); append(RET); blocks[0x6f] = blocks[0x1a] = blocks[0x7a] = define(); // 40
  append(LIT); append(DIG4); append(DIG5); append(RET); blocks[0x06] = blocks[0x69] = define(); // 45
  append(LIT); append(DIG5); append(DIG0); append(RET); blocks[0x6c] = blocks[0x3c] = define(); // 50
  append(LIT); append(DIG6); append(DIG0); append(RET); blocks[0x04] = blocks[0x6a] = define(); // 60
  append(LIT); append(DIG7); append(DIG0); append(RET); blocks[0x6d] = blocks[0x2a] = define(); // 70
  append(LIT); append(DIG7); append(DIG2); append(RET); blocks[0x68] = blocks[0x20] = define(); // 72
  append(LIT); append(DIG8); append(DIG0); append(RET); blocks[0x60] = blocks[0x30] = define(); // 80
  append(LIT); append(DIG9); append(DIG0); append(RET); blocks[0x10] = blocks[0x08] = blocks[0x34] = define(); // 90
  append(LIT); append(DIG1); append(DIG8); append(DIG0); append(RET); blocks[0x32] = blocks[0x0e] = define(); // 180
  append(LIT); append(DIG1); append(DIG2); append(DIG0); append(RET); blocks[0x3a] = blocks[0x61] = define(); // 120

  delay(2000);
  Serial.println("INITIALIZED (NORMAL MODE)");
#endif
}

void manual(int x, int y) {
  int dx = x > 700 ? 1 : x < 300 ? -1 : 0;
  int dy = y > 700 ? -1 : y < 300 ? 1 : 0;
  if (dx != 0 || dy != 0) {
    moveCNC(dx, dy);
    theta = x = y = lastX = lastY = 0.0;
  }
}

bool latch = false;
void loop() {
  manual(analogRead(X_PIN), analogRead(Y_PIN));
  int button = digitalRead(BUTTON);
  if (!latch && button == LOW) {
    latch = true;
    Serial.println("");
    Serial.println("-----------------------------------------------------------------------");
    Serial.println("READ PROGRAM");
    readBlocks();
  } else {
    if (button == LOW) {
      latch = false;
    }
  }
}
