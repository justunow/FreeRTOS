/* FreeRTOS mutex demo */

#if CONFIG_FREERTOS_UNICORE
static const BaseType_t app_cpu = 0;
#else
static const BaseType_t app_cpu = 1;
#endif

static int shared_var = 0;
static SemaphoreHandle_t mutex;

void incTask(void *parameters) {
  int local_var;
  while (1) {
    // take mutex prior to critical section,
    // check for and take mutex as an atomic action
    if (xSemaphoreTake(mutex, 0) == pdTRUE) {
      // critical section: works with shared resources, excludes other tasks from
      // entering a critical section is known as mutual exclusion (mutex), equals
      // to lock which only allows one thread to enter the locked section of code
      local_var = shared_var;
      local_var++;
      vTaskDelay(random(100, 500) / portTICK_PERIOD_MS);
      shared_var = local_var;

      xSemaphoreGive(mutex);

      Serial.println(shared_var);
    }
    else {
      // do something else if can not obtain mutex
    }
  }
}

void setup() {
  randomSeed(analogRead(0));
  Serial.begin(115200);
  Serial.println("---FreeRTOS Mutex Demo---");

  // create mutex before starting tasks
  mutex = xSemaphoreCreateMutex();

  xTaskCreatePinnedToCore(incTask, "Increment Task 1", 1024, NULL, 1, NULL, app_cpu);
  xTaskCreatePinnedToCore(incTask, "Increment Task 2", 1024, NULL, 1, NULL, app_cpu);

  vTaskDelete(NULL);
}

void loop() {
}
