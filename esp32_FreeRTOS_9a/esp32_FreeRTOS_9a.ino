/* FreeRTOS hardware interrupts demo - critical section */

#if CONFIG_FREERTOS_UNICORE
static const BaseType_t app_cpu = 0;
#else
static const BaseType_t app_cpu = 1;
#endif

static const uint16_t timer_divider = 8;
static const uint64_t timer_max_count = 1000000;
static const TickType_t task_delay = 2000 / portTICK_PERIOD_MS;

static hw_timer_t *timer = NULL;
static volatile int isr_counter;
static portMUX_TYPE spinlock = portMUX_INITIALIZER_UNLOCKED;

/* interrupt service routines (ISRs) */
void IRAM_ATTR onTimer() {
  portENTER_CRITICAL_ISR(&spinlock);
  isr_counter++;
  portEXIT_CRITICAL_ISR(&spinlock);
}

void printValues(void *parameters) {
  while (1) {
    while (isr_counter > 0) {
      Serial.println(isr_counter);

      portENTER_CRITICAL(&spinlock);
      isr_counter--;
      portEXIT_CRITICAL(&spinlock);
    }
    vTaskDelay(task_delay);
  }
}

void setup() {
  Serial.begin(115200);
  Serial.println("---FreeRTOS ISR Critical Section Demo---");

  xTaskCreatePinnedToCore(printValues, "Print Values", 1024, NULL, 1, NULL, app_cpu);

  // create and start timer (num, divider, count-up)
  timer = timerBegin(0, timer_divider, true);

  // provide ISR to timer (timer, function, edge)
  timerAttachInterrupt(timer, &onTimer, true);

  // at what count should ISR trigger (timer, count, auto-reload)
  timerAlarmWrite(timer, timer_max_count, true);

  // allow ISR to trigger
  timerAlarmEnable(timer);

  vTaskDelete(NULL);
}

void loop() {
}
