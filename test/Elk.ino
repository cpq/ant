extern "C" {
#include "elk.h"
}

void setup() {
  Serial.begin(115200);
}

static long exec_elk(void) {
  char buf[200];
  struct js *js = js_create(buf, sizeof(buf));
  jsval_t res =
      js_eval(js, "let a=0, i=0; while(i++ < 999) a += i + i / 3; a", ~0);
  return atol(js_str(js, res));
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
  measure_time("elk", exec_elk);
  measure_time("c  ", exec_c);
  delay(1000);
}
