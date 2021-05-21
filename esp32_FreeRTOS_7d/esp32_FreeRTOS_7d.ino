/* FreeRTOS semaphore - queue */

#if CONFIG_FREERTOS_UNICORE
static const BaseType_t app_cpu = 0;
#else
static const BaseType_t app_cpu = 1;
#endif

static const uint8_t queue_len = 10;  // size of queue
static const int num_prod_tasks = 5;  // number of producer tasks
static const int num_cons_tasks = 2;  // number of consumer tasks
static const int num_writes = 3;      // num times each producer writes to buf

static SemaphoreHandle_t bin_sem;     // waits for parameter to be read
static SemaphoreHandle_t mutex;       // lock access to serial resource
static QueueHandle_t msg_queue;       // send data from producer to consumer

// producer: write a given number of times to shared buffer
void producer(void *parameters) {
  int num = *(int *)parameters;

  xSemaphoreGive(bin_sem);

  // fill queue with task number (wait max time if queue is full)
  for (int i = 0; i < num_writes; i++) {
    xQueueSend(msg_queue, (void *)&num, portMAX_DELAY);
  }

  vTaskDelete(NULL);
}

// consumer: continuously read from shared buffer
void consumer(void *parameters) {
  int val;

  // read from buffer
  while (1) {
    // read from queue (wait max time if queue is empty)
    xQueueReceive(msg_queue, (void *)&val, portMAX_DELAY);

    xSemaphoreTake(mutex, portMAX_DELAY);
    Serial.println(val);
    xSemaphoreGive(mutex);
  }
}

void setup() {
  char task_name[12];
  Serial.begin(115200);

  bin_sem = xSemaphoreCreateBinary();
  mutex = xSemaphoreCreateMutex();
  msg_queue = xQueueCreate(queue_len, sizeof(int));

  // start producer tasks (wait for each to read argument)
  for (int i = 0; i < num_prod_tasks; i++) {
    sprintf(task_name, "Producer %i", i);
    xTaskCreatePinnedToCore(producer, task_name, 1024, (void *)&i, 1, NULL, app_cpu);
    xSemaphoreTake(bin_sem, portMAX_DELAY);
  }

  // start consumer tasks
  for (int i = 0; i < num_cons_tasks; i++) {
    sprintf(task_name, "Consumer %i", i);
    xTaskCreatePinnedToCore(consumer, task_name, 1024, NULL, 1, NULL, app_cpu);
  }

  Serial.println("All tasks created!");
}

void loop() {
}
