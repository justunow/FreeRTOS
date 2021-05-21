/* FreeRTOS muliticore systems demo - spinlock */

static const BaseType_t pro_cpu = 0;
static const BaseType_t app_cpu = 1;

static const TickType_t time_hog = 1; // should be as short as possible in the critical section
static const TickType_t task_0_delay = 30;
static const TickType_t task_1_delay = 100;

static const int pin_0 = 12;
static const int pin_1 = 13;

static portMUX_TYPE spinlock = portMUX_INITIALIZER_UNLOCKED;

// hogs the processor. Accurate to about 1 second (no promises).
static void hog_delay(uint32_t ms) {
  for (uint32_t i = 0; i < ms; i++) {
    for (uint32_t j = 0; j < 40000; j++) {
      asm("nop");
    }
  }
}

// task in Core 0
void doTask0(void *parameters) {
  pinMode(pin_0, OUTPUT);
  while (1) {
    portENTER_CRITICAL(&spinlock);
    digitalWrite(pin_0, !digitalRead(pin_0));
    portEXIT_CRITICAL(&spinlock);

    vTaskDelay(task_0_delay / portTICK_PERIOD_MS);
  }
}

// task in Core 1
void doTask1(void *parameters) {
  pinMode(pin_1, OUTPUT);
  while (1) {
    portENTER_CRITICAL(&spinlock);
    digitalWrite(pin_1, HIGH);
    hog_delay(time_hog);
    digitalWrite(pin_1, LOW);
    portEXIT_CRITICAL(&spinlock);

    vTaskDelay(task_1_delay / portTICK_PERIOD_MS);
  }
}

void setup() {
  xTaskCreatePinnedToCore(doTask0, "Task 0", 1024, NULL, 1, NULL, pro_cpu); // tskNO_AFFINITY);
  xTaskCreatePinnedToCore(doTask1, "Task 1", 1024, NULL, 1, NULL, app_cpu); // tskNO_AFFINITY);

  vTaskDelete(NULL);
}

void loop() {
}
