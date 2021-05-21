/* FreeRTOS deadlock - hierarchy */

#if CONFIG_FREERTOS_UNICORE
static const BaseType_t app_cpu = 0;
#else
static const BaseType_t app_cpu = 1;
#endif

enum { NUM_TASKS = 5 };
enum { TASK_STACK_SIZE = 2048 };  // bytes in ESP32, words in vanilla FreeRTOS

static SemaphoreHandle_t bin_sem;
static SemaphoreHandle_t done_sem;
static SemaphoreHandle_t chopstick[NUM_TASKS];

void eat(void *parameters) {
  int num;
  char buf[50];

  num = *(int *)parameters;
  xSemaphoreGive(bin_sem);

  // take left chopstick
  xSemaphoreTake(chopstick[num], portMAX_DELAY);
  sprintf(buf, "Philosopher %i took chopstick %i", num, num);
  Serial.println(buf);

  //vTaskDelay(1 / portTICK_PERIOD_MS);

  // take right chopstick
  xSemaphoreTake(chopstick[(num + 1) % NUM_TASKS], portMAX_DELAY);
  sprintf(buf, "Philosopher %i took chopstick %i", num, (num + 1) % NUM_TASKS);
  Serial.println(buf);

  vTaskDelay(1 / portTICK_PERIOD_MS);

  // do some eating
  sprintf(buf, "Philosopher %i is eating", num);
  Serial.println(buf);
  vTaskDelay(10 / portTICK_PERIOD_MS);

  // put down right chopstick
  xSemaphoreGive(chopstick[(num + 1) % NUM_TASKS]);
  sprintf(buf, "Philosopher %i returned chopstick %i", num, (num + 1) % NUM_TASKS);
  Serial.println(buf);

  // put down left chopstick
  xSemaphoreGive(chopstick[num]);
  sprintf(buf, "Philosopher %i returned chopstick %i", num, num);
  Serial.println(buf);

  // notify main task and delete self
  xSemaphoreGive(done_sem);
  vTaskDelete(NULL);
}

void setup() {
  char task_name[20];

  Serial.begin(115200);

  bin_sem = xSemaphoreCreateBinary();
  done_sem = xSemaphoreCreateCounting(NUM_TASKS, 0);
  for (int i = 0; i < NUM_TASKS; i++) {
    chopstick[i] = xSemaphoreCreateMutex();
  }

  for (int i = 0; i < NUM_TASKS; i++) {
    sprintf(task_name, "Philosopher %i", i);
    xTaskCreatePinnedToCore(eat, task_name, TASK_STACK_SIZE, (void *)&i, 1, NULL, app_cpu);
    xSemaphoreTake(bin_sem, portMAX_DELAY);
  }

  for (int i = 0; i < NUM_TASKS; i++) {
    xSemaphoreTake(done_sem, portMAX_DELAY);
  }

  Serial.println("Done! No deadlock occurred!");
}

void loop() {
}
