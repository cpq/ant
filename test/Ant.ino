#include "ant.h"

void setup() {
  Serial.begin(115200);
}

static long exec_ant(void) {
  struct ant ant = ANT_INITIALIZER;
  return ant_eval(&ant,
                  "a=0; i=0; b=1; c=1000; # a += i+i/3; i += b; @b i<c; a");
}

static long exec_ant2(void) {
  struct ant2 ant = ANT2_INITIALIZER;
  return ant2_eval(&ant, "0=a 0=i 1000=d  # ai+i3/+=a Ii id< @b a");
}

static long exec_ant3(void) {
  struct ant3 ant = {{3, 1000}, {0}, {0}, 0};
  unsigned char code[] = {PushVar, 0,       PushVar, 1,         Plus, PushVar,
                          1,       PushImm, 0,       Div,       Plus, PopVar,
                          0,       IncVar,  1,       CmpVarImm, 1,    1,
                          Jump,    0,       PushVar, 0,         Done};
  return ant3_eval(&ant, code);
}

static long exec_antx(void) {
  struct ant3 ant = {{3, 1000}, {0}, {0}, 0};
  unsigned char code[] = {PushVar, 0,       PushVar, 1,         Plus, PushVar,
                          1,       PushImm, 0,       Div,       Plus, PopVar,
                          0,       IncVar,  1,       CmpVarImm, 1,    1,
                          Jump,    0,       PushVar, 0,         Done};
  return ant3_eval2(&ant, code);
}

static long exec_c(void) {
  long res = 0;
  for (long i = 0; i < 1000; i++) res += i + i / 3;
  return res;
}

static void measure_time(const char *tag, long (*fn)(void)) {
  unsigned long start = micros();
  long res = fn();
  unsigned long duration = micros() - start;
  Serial.print(tag);
  Serial.print(", result: ");
  Serial.print(res);
  Serial.print(", microseconds: ");
  Serial.println(duration);
}

void loop() {
  Serial.print("Ant size: ");
  Serial.println(sizeof(struct ant));
  measure_time(" ant", exec_ant);
  measure_time("ant2", exec_ant2);
  measure_time("ant3", exec_ant3);
  measure_time("antx", exec_ant3);
  measure_time("   c", exec_c);
  delay(1000);
}
