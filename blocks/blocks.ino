#define MEM_SIZE 512
#define MAX_DATA_STACK 8
#define MAX_RETURN_STACK 8
#define MAX_PRIMITIVES 128

uint8_t memory[MEM_SIZE];
uint8_t mem(int16_t address) { return memory[address]; } // TODO: bounds check
void memset(int16_t address, uint8_t value) { memory[address] = value; } // TODO: bounds check

int16_t dstack[MAX_DATA_STACK];
int16_t* s;
void push(int16_t i) { *(++s) = i; } // TODO: overflow check
int16_t pop() { return *s--; } // TODO: underflow check

int16_t rstack[MAX_RETURN_STACK];
int16_t* r;
void rpush(int16_t i) { *(++r) = i; } // TODO: overflow check
int16_t rpop() { return *r--; } // TODO: underflow check

void (*instructions[MAX_PRIMITIVES])(); // instruction function table
void bind(uint8_t i, void (*f)()) { instructions[i] = f; }

int16_t p; // program counter (VM instruction pointer)

void ret() { Serial.println("RETURN"); p = rpop(); }

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

void add() { Serial.println("ADD"); push(pop() + pop()); }
void sub() { Serial.println("SUB"); push(pop() - pop()); }
void mul() { Serial.println("MUL"); push(pop() * pop()); }
void div() { Serial.println("DIV"); push(pop() / pop()); }
void mod() { Serial.println("MOD"); push(pop() % pop()); }
void neg() { Serial.println("NEG"); push(-pop()); }

double theta = 0.0;
double x = 0.0;
double y = 0.0;

void pose() { Serial.print("Pose: "); Serial.print((int)(theta / PI * 180.0 + 0.5)); Serial.print(" "); Serial.print((int)(x + 0.5)); Serial.print(" "); Serial.println((int)(y + 0.5)); }

void go() { Serial.println("GO"); y = pop(); x = pop(); pose(); }
void head() { Serial.println("HEAD"); theta = pop(); pose();}
void center() { Serial.println(":CENTER"); lit(); lit(); lit(); go(); head(); } // TODO: secondary word?
void turn() { Serial.println("TURN"); theta += pop() / 180.0 * PI; pose(); }
void left() { Serial.println(":LEFT"); neg(); turn(); } // TODO: secondary word?
void right() { Serial.println(":RIGHT"); turn(); } // TODO: secondary word?
void forward() { Serial.println("FORWARD"); double d = pop(); x += d * cos(theta); y += d * sin(theta); pose(); }
void back() { Serial.println(":BACK"); neg(); forward(); } // TODO: secondary word?

void run() {
  int16_t i;
  do {
    i = mem(p++);
    if ((i & 0x80) == 0) { // instruction?
      instructions[i](); // execute
    } else { // address to call
      if (mem(p + 1) != 0) rpush(p + 1); // push return address if not followed by return (TCO)
      p = ((i << 8) & 0x7f00) | mem(p); // jump
    }
  } while (p >= 0); // note: -1 initially pushed to return stack
}

void exec(int16_t address) {
  r = rstack - 1; // reset return stack
  p = address;
  rpush(-1); // causing run() to fall through upon completion
  run();
}

bool readBlocks() {
  int count = 0;
  while (count < MEM_SIZE && Serial.available() > 0) { // TODO: error when exceeding MAX_INSTRUCTIONS?
    byte b = Serial.read();
    memset(count++, b);
    Serial.print("Read: ");
    Serial.println(b, DEC);
    delay(50); // enforce reading in batch
  }
  memset(count, 0);
  return count > 0;
}

void setup() {
  Serial.begin(9600);
  bind(0,   ret);     // 
  bind(1,   nop);     // 
  bind(2,   nop);     // 
  bind(3,   nop);     // 
  bind(4,   nop);     // 
  bind(5,   nop);     // 
  bind(6,   nop);     // 
  bind(7,   nop);     // 
  bind(8,   nop);     // 
  bind(9,   nop);     // 
  bind(10,  nop);     // 
  bind(11,  nop);     // 
  bind(12,  nop);     // 
  bind(13,  nop);     // 
  bind(14,  nop);     // 
  bind(15,  nop);     // 
  bind(16,  nop);     // 
  bind(17,  nop);     // 
  bind(18,  nop);     // 
  bind(19,  nop);     // 
  bind(20,  nop);     // 
  bind(21,  nop);     // 
  bind(22,  nop);     // 
  bind(23,  nop);     // 
  bind(24,  nop);     // 
  bind(25,  nop);     // 
  bind(26,  nop);     // 
  bind(27,  nop);     // 
  bind(28,  nop);     // 
  bind(29,  nop);     // 
  bind(30,  nop);     // 
  bind(31,  nop);     // 
  bind(32,  nop);     // 
  bind(33,  nop);     // 
  bind(34,  nop);     // 
  bind(35,  lit);     // #
  bind(36,  nop);     // 
  bind(37,  mod);     // %
  bind(38,  nop);     // 
  bind(39,  nop);     // 
  bind(40,  nop);     // 
  bind(41,  nop);     // 
  bind(42,  mul);     // *
  bind(43,  add);     // +
  bind(44,  nop);     // 
  bind(45,  sub);     // -
  bind(46,  show);    // .
  bind(47,  div);     // /
  bind(48,  zero);    // 0
  bind(49,  one);     // 1
  bind(50,  two);     // 2
  bind(51,  three);   // 3
  bind(52,  four);    // 4
  bind(53,  five);    // 5
  bind(54,  six);     // 6
  bind(55,  seven);   // 7
  bind(56,  eight);   // 8
  bind(57,  nine);    // 9
  bind(58,  nop);     // 
  bind(59,  nop);     // 
  bind(60,  nop);     // 
  bind(61,  nop);     // 
  bind(62,  nop);     // 
  bind(63,  nop);     // 
  bind(64,  nop);     // 
  bind(65,  nop);     // 
  bind(66,  nop);     // 
  bind(67,  nop);     // 
  bind(68,  nop);     // 
  bind(69,  nop);     // 
  bind(70,  nop);     // 
  bind(71,  nop);     // 
  bind(72,  nop);     // 
  bind(73,  nop);     // 
  bind(74,  nop);     // 
  bind(75,  nop);     // 
  bind(76,  nop);     // 
  bind(77,  nop);     // 
  bind(78,  nop);     // 
  bind(79,  nop);     // 
  bind(80,  nop);     // 
  bind(81,  nop);     // 
  bind(82,  nop);     // 
  bind(83,  nop);     // 
  bind(84,  nop);     // 
  bind(85,  nop);     // 
  bind(86,  nop);     // 
  bind(87,  nop);     // 
  bind(88,  nop);     // 
  bind(89,  nop);     // 
  bind(90,  nop);     // 
  bind(91,  nop);     // 
  bind(92,  nop);     // 
  bind(93,  nop);     // 
  bind(94,  nop);     // 
  bind(95,  nop);     // 
  bind(96,  nop);     // 
  bind(97,  nop);     // 
  bind(98,  back);    // b
  bind(99,  center);  // c
  bind(100, nop);     // 
  bind(101, nop);     // 
  bind(102, forward); // f
  bind(103, go);      // g
  bind(104, head);    // h
  bind(105, nop);     // 
  bind(106, nop);     // 
  bind(107, nop);     // 
  bind(108, left);    // l
  bind(109, nop);     // 
  bind(110, nop);     // 
  bind(111, nop);     // 
  bind(112, nop);     // 
  bind(113, nop);     // 
  bind(114, right);   // r
  bind(115, nop);     // 
  bind(116, turn);    // t
  bind(117, nop);     // 
  bind(118, nop);     // 
  bind(119, nop);     // 
  bind(120, nop);     // 
  bind(121, nop);     // 
  bind(122, nop);     // 
  bind(123, nop);     // 
  bind(124, nop);     // 
  bind(125, nop);     // 
  bind(126, nop);     // 
  bind(127, nop);     // 
}

void loop() {
  if (readBlocks()) {
    Serial.println("Execute blocks");
    exec(0);
  }
}
