#include "ant.h"

void setup() {
  Serial.begin(115200);
}

static long exec_ant(void) {
  struct ant ant;
  ant_init(&ant);
  return ant_eval(&ant, "a=0; i=0; # a += i + i / 3; i += 1; @b i<1000; a");
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
  measure_time("ant", exec_ant);
  measure_time("c  ", exec_c);
  delay(1000);
}
