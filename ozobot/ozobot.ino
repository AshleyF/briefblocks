#include <Arduino.h>

#define RED   9
#define GREEN 6
#define BLUE  5

void led(bool red, bool green, bool blue) {
  analogWrite(RED,   red   ? 128 : 255);
  analogWrite(GREEN, green ? 0 : 255);
  analogWrite(BLUE,  blue  ? 43 : 255);
  delay(50);
}

void K() { led(false, false, false); }
void R() { led( true, false, false); }
void G() { led(false,  true, false); }
void B() { led(false, false,  true); }
void C() { led(false,  true,  true); }
void M() { led( true, false,  true); }
void Y() { led( true,  true, false); }
void W() { led( true,  true,  true); }

void programOzobot() {
  C();
  R();
  Y();
  C();
  Y();
  M();
  C();
  R();
  W();
  K();
  W();
  R();
  K();
  W();
  Y();
  B();
  K();
  W();
  K();
  W();
  K();
  W();
  Y();
  G();
  K();
  C();
  Y();
  K();
  M();
  R();
  Y();
  K();
  W();
  G();
  B();
  R();
  K();
  W();
  K();
  W();
  K();
  W();
  Y();
  M();
  G();
  W();
  K();
  G();
  Y();
  R();
  W();
  K();
  W();
  K();
  G();
  B();
  R();
  K();
  W();
  K();
  Y();
  M();
  G();
  W();
  K();
  G();
  Y();
  R();
  W();
  K();
  W();
  K();
  W();
  K();
  W();
  G();
  B();
  R();
  Y();
  M();
  G();
  W();
  K();
  G();
  Y();
  R();
  W();
  K();
  W();
  K();
  Y();
  W();
  C();
  B();
  M();
  C();
  W();
  M();
  W();
}

void test()
{
  for(int r = 0; r < 255; r++)
  {
    for(int g = 0; g < 255; g++)
    {
      for (int b = 0; b < 255; b++)
      {
        analogWrite(RED,   255 - r);
        analogWrite(GREEN, 255 - g);
        analogWrite(BLUE,  255 - b);
        //delay(0);
      }
    }
  }
}

void calibrate()
{
  W();
  delay(1000);
  R();
  delay(1000);
  G();
  delay(1000);
  B();
  delay(1000);
  C();
  delay(1000);
  M();
  delay(1000);
  Y();
  delay(1000);
  K();
  delay(1000);
}

void setup()
{
  pinMode(RED,   OUTPUT);
  pinMode(GREEN, OUTPUT);
  pinMode(BLUE,  OUTPUT);
  W();
  delay(8000);
  programOzobot();
  //test();
  W();
}

void loop() {
  calibrate();
}

