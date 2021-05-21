/* FreeRTOS hardware interrupts demo - binary semaphore */

#if CONFIG_FREERTOS_UNICORE
static const BaseType_t app_cpu = 0;
#else
static const BaseType_t app_cpu = 1;
#endif

static const uint16_t timer_divider = 80;
static const uint64_t timer_max_count = 1000000;

static const int adc_pin = A0;

static hw_timer_t *timer = NULL;
static volatile uint16_t val;
static SemaphoreHandle_t bin_sem = NULL;

// this function executes when timer reaches max (and resets)
void IRAM_ATTR onTimer() {
  BaseType_t task_woken = pdFALSE;

  // perform action (read from ADC)
  val = analogRead(adc_pin);

  // give semaphore to tell task that new value is ready
  xSemaphoreGiveFromISR(bin_sem, &task_woken);

  // exit from ISR (ESP-IDF)
  if (task_woken) {
    portYIELD_FROM_ISR();
  }
}

// wait for semaphore and print out ADC value when received
void printValues(void *parameters) {
  while (1) {
    xSemaphoreTake(bin_sem, portMAX_DELAY);
    Serial.println(val);
  }
}

void setup() {
  Serial.begin(115200);
  Serial.println("---FreeRTOS ISR Buffer Demo---");

  // create semaphore before it is used (in task or ISR)
  bin_sem = xSemaphoreCreateBinary();

  // force reboot if we can not create the semaphore
  if (bin_sem == NULL) {
    Serial.println("Could not create semaphore");
    ESP.restart();
  }

  xTaskCreatePinnedToCore(printValues, "Print Values", 1024, NULL, 2, NULL, app_cpu);

  // create and start timer (num, divider, count-up)
  timer = timerBegin(0, timer_divider, true);

  // provide ISR to timer (timer, function, edge)
  timerAttachInterrupt(timer, &onTimer, true);

  // at what count should ISR trigger (timer, count, auto-reload)
  timerAlarmWrite(timer, timer_max_count, true);

  // allow ISR to trigger
  timerAlarmEnable(timer);
}

void loop() {
}
