/* FreeRTOS hardware interrupts */

#if CONFIG_FREERTOS_UNICORE
static const BaseType_t app_cpu = 0;
#else
static const BaseType_t app_cpu = 1;
#endif

static const uint16_t timer_divider = 8;
static const uint64_t timer_max_count = 1000000;
static const int adc = A0;
static const int sampleNum = 10;

static hw_timer_t *timer = NULL;
static SemaphoreHandle_t bin_sem = NULL;

static volatile uint16_t buf[sampleNum];
int count = 0;
float avg;

void IRAM_ATTR onTimer() {
  BaseType_t task_woken = pdFALSE;

  buf[count] = analogRead(adc);
  count++;
  if (count == sampleNum) {
    count = 0;
    xSemaphoreGiveFromISR(bin_sem, &task_woken);
  }

  if (task_woken) {
    portYIELD_FROM_ISR();
  }
}

// task A: computes average of 10 samples
void computeAvg(void *parameters) {
  while (1) {
    xSemaphoreTake(bin_sem, portMAX_DELAY);
    int sum = 0;
    for (int i = 0; i < sizeof(buf) / sizeof(buf[0]); i++) {
      sum += buf[i];
    }
    avg = (double)sum / sampleNum;
  }
}

// task B: echoes back the serial input and prints the average value of samples
void readSerial(void *parameters) {
  String incomingBytes;
  while (1) {
    if (Serial.available()) {
      incomingBytes = Serial.readStringUntil('\n');
      Serial.println(incomingBytes);
      if (incomingBytes.substring(0, 3) == "avg") {
        Serial.print("Average:\t");
        Serial.println(avg);
      }
    }
  }
}

void setup() {
  Serial.begin(115200);

  bin_sem = xSemaphoreCreateBinary();
  if (bin_sem == NULL) {
    Serial.println("Could not create semaphore");
    ESP.restart();
  }

  xTaskCreatePinnedToCore(computeAvg, "Compute Average", 1024, NULL, 2, NULL, app_cpu);
  xTaskCreatePinnedToCore(readSerial, "Read Serial", 1024, NULL, 1, NULL, app_cpu);

  timer = timerBegin(0, timer_divider, true);
  timerAttachInterrupt(timer, &onTimer, true);
  timerAlarmWrite(timer, timer_max_count, true);
  timerAlarmEnable(timer);

  vTaskDelete(NULL);
}

void loop() {
}
