/* FreeRTOS software timer */

#if CONFIG_FREERTOS_UNICORE
static const BaseType_t app_cpu = 0;
#else
static const BaseType_t app_cpu = 1;
#endif

static const int led = LED_BUILTIN;
static TimerHandle_t fiveSecondTimer = NULL;

void myTimerCallback(TimerHandle_t xTimer) {
  digitalWrite(led, LOW);
}

void readSerial(void *parameters) {
  while (1) {
    if (Serial.available()) {
      Serial.print((char)Serial.read());
      digitalWrite(led, HIGH);
      xTimerStart(fiveSecondTimer, portMAX_DELAY);
    }
  }
}

void setup() {
  pinMode(led, OUTPUT);
  Serial.begin(115200);

  fiveSecondTimer = xTimerCreate("Five-second Timer", 5000 / portTICK_PERIOD_MS, \
                                 pdFALSE, (void *)0, myTimerCallback);

  xTaskCreatePinnedToCore(readSerial, "Read Serial", 1024, NULL, 1, NULL, app_cpu);

  if (fiveSecondTimer == NULL) {
    Serial.println("Could not create one of the timers");
  }

  vTaskDelete(NULL);
}

void loop() {
}
