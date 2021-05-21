/* FreeRTOS semaphore - counting semaphore */

#if CONFIG_FREERTOS_UNICORE
static const BaseType_t app_cpu = 0;
#else
static const BaseType_t app_cpu = 1;
#endif

enum {BUF_SIZE = 5};                  // size of buffer array
static const int num_prod_tasks = 5;  // number of producer tasks
static const int num_cons_tasks = 2;  // number of consumer tasks
static const int num_writes = 3;      // num times each producer writes to buf
static int buf[BUF_SIZE];             // shared buffer
static int head = 0;                  // writing index to buffer
static int tail = 0;                  // reading index to buffer

static SemaphoreHandle_t bin_sem;
static SemaphoreHandle_t sem_filled;
static SemaphoreHandle_t sem_empty;
static SemaphoreHandle_t mutex;

// producer: write a given number of times to shared buffer
void producer(void *parameters) {
  int num = *(int *)parameters;

  xSemaphoreGive(bin_sem);

  // fill shared buffer with task number
  for (int i = 0; i < num_writes; i++) {
    // wait for empty slot in buffer to be available
    xSemaphoreTake(sem_empty, portMAX_DELAY);

    // lock critical section with a mutex
    xSemaphoreTake(mutex, portMAX_DELAY);
    buf[head] = num;
    head = (head + 1) % BUF_SIZE;
    xSemaphoreGive(mutex);

    xSemaphoreGive(sem_filled);
  }

  vTaskDelete(NULL);
}

// consumer: continuously read from shared buffer
void consumer(void *parameters) {
  int val;

  // read from buffer
  while (1) {
    // wait for at least one slot in buffer to be filled
    xSemaphoreTake(sem_filled, portMAX_DELAY);

    // lock critical section with a mutex
    xSemaphoreTake(mutex, portMAX_DELAY);
    val = buf[tail];
    tail = (tail + 1) % BUF_SIZE;
    Serial.println(val);
    xSemaphoreGive(mutex);

    xSemaphoreGive(sem_empty);
  }
}

void setup() {
  char task_name[12];
  Serial.begin(115200);

  bin_sem = xSemaphoreCreateBinary();
  sem_filled = xSemaphoreCreateCounting(BUF_SIZE, 0);
  sem_empty = xSemaphoreCreateCounting(BUF_SIZE, BUF_SIZE);
  mutex = xSemaphoreCreateMutex();

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
