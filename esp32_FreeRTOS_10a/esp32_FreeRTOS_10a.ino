/* FreeRTOS deadlock demo - timeout */

#if CONFIG_FREERTOS_UNICORE
static const BaseType_t app_cpu = 0;
#else
static const BaseType_t app_cpu = 1;
#endif

TickType_t mutex_timeout = 1000 / portTICK_PERIOD_MS;

static SemaphoreHandle_t mutex_1;
static SemaphoreHandle_t mutex_2;

// task A (high priority)
void doTaskA(void *parameter) {
  while (1) {
    if (xSemaphoreTake(mutex_1, mutex_timeout) == pdTRUE) {
      Serial.println("Task A took mutux 1");
      // force the deadlock to happen
      vTaskDelay(1 / portTICK_PERIOD_MS);

      if (xSemaphoreTake(mutex_2, mutex_timeout) == pdTRUE) {
        Serial.println("Task A took mutex 2");

        Serial.println("Task A doing some work");
        vTaskDelay(500 / portTICK_PERIOD_MS);
      }
      else {
        Serial.println("Task A timed out waiting for mutex 2");
      }
    }
    else {
      Serial.println("Task A timed out waiting for mutex 1");
    }
    xSemaphoreGive(mutex_2);
    xSemaphoreGive(mutex_1);

    Serial.println("Task A going to sleep");
    vTaskDelay(500 / portTICK_PERIOD_MS);
  }
}

// task B (low priority)
void doTaskB(void *parameter) {
  while (1) {
    if (xSemaphoreTake(mutex_2, mutex_timeout) == pdTRUE) {
      Serial.println("Task B took mutux 2");
      // force the deadlock to happen
      vTaskDelay(1 / portTICK_PERIOD_MS);

      if (xSemaphoreTake(mutex_1, mutex_timeout) == pdTRUE) {
        Serial.println("Task B took mutex 1");

        Serial.println("Task B doing some work");
        vTaskDelay(500 / portTICK_PERIOD_MS);
      }
      else {
        Serial.println("Task B timed out waiting for mutex 1");
      }
    }
    else {
      Serial.println("Task B timed out waiting for mutex 2");
    }
    xSemaphoreGive(mutex_1);
    xSemaphoreGive(mutex_2);

    Serial.println("Task B going to sleep");
    vTaskDelay(500 / portTICK_PERIOD_MS);
  }
}

void setup() {
  Serial.begin(115200);
  Serial.println("---FreeRTOS Deadlock Demo---");

  mutex_1 = xSemaphoreCreateMutex();
  mutex_2 = xSemaphoreCreateMutex();

  xTaskCreatePinnedToCore(doTaskA, "Task A", 1024, NULL, 2, NULL, app_cpu);
  xTaskCreatePinnedToCore(doTaskB, "Task B", 1024, NULL, 1, NULL, app_cpu);

  vTaskDelete(NULL);
}

void loop() {
}
