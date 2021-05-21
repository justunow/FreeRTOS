/* FreeRTOS muliticore systems demo - semaphore */

static const BaseType_t pro_cpu = 0;
static const BaseType_t app_cpu = 1;

static const uint32_t task_0_delay = 500;
static const int led_pin = LED_BUILTIN;

static SemaphoreHandle_t bin_sem;

// task in core 0
void doTask0(void *parameters) {
  pinMode(led_pin, OUTPUT);
  while (1) {
    xSemaphoreGive(bin_sem);
    vTaskDelay(task_0_delay / portTICK_PERIOD_MS);
  }
}

// task in core 1
void doTask1(void *parameters) {
  while (1) {
    xSemaphoreTake(bin_sem, portMAX_DELAY);
    digitalWrite(led_pin, !digitalRead(led_pin));
  }
}

void setup() {
  bin_sem = xSemaphoreCreateBinary();

  xTaskCreatePinnedToCore(doTask0, "Task 0", 1024, NULL, 1, NULL, pro_cpu); // tskNO_AFFINITY);
  xTaskCreatePinnedToCore(doTask1, "Task 1", 1024, NULL, 2, NULL, app_cpu); // tskNO_AFFINITY);

  vTaskDelete(NULL);
}

void loop() {
}
