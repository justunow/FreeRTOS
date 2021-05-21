/* FreeRTOS task scheduling */

#if CONFIG_FREERTOS_UNICORE
static const BaseType_t app_cpu = 0;
#else
static const BaseType_t app_cpu = 1;
#endif

static const int led = LED_BUILTIN;

unsigned long blinkPeriod = 1000;

static const uint8_t buf_len = 20;

/* task 1: blink a led */
void blinkLed(void *parameter) {
  while (1) {
    digitalWrite(led, HIGH);
    /* vTaskDelay() is non-blocking, tells the mcu
      to run other tasks until the delay time is up */
    vTaskDelay(blinkPeriod / portTICK_PERIOD_MS);
    digitalWrite(led, LOW);
    vTaskDelay(blinkPeriod / portTICK_PERIOD_MS);
  }
}

/* task 2: listen to the serial input */
void blinkRate(void *parameter) {
  char c;
  char buf[buf_len];
  uint8_t idx = 0;

  memset(buf, 0, buf_len);
  while (1) {
    if (Serial.available()) {
      c = Serial.read();
      if (c == '\n') {
        blinkPeriod = atoi(buf);
        Serial.print("Updated LED delay to:\t");
        Serial.print(blinkPeriod);
        Serial.println("\tms");
        memset(buf, 0, buf_len);
        idx = 0;
      }
      else {
        if (idx < buf_len - 1) {
          buf[idx] = c;
          idx++;
        }
      }
    }
  }
}

void setup() {
  pinMode(led, OUTPUT);
  Serial.begin(300);

  xTaskCreatePinnedToCore(blinkLed, "blink", 1024, NULL, 1, NULL, app_cpu);
  xTaskCreatePinnedToCore(blinkRate, "rate", 1024, NULL, 1, NULL, app_cpu);

  vTaskDelete(NULL);
}

void loop() {
}
