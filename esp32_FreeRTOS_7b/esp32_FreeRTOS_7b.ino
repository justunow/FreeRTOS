/* FreeRTOS semaphore demo - counting semahpore */

#if CONFIG_FREERTOS_UNICORE
static const BaseType_t app_cpu = 0;
#else
static const BaseType_t app_cpu = 1;
#endif

// counts down when parameters read
static SemaphoreHandle_t count_sem;
static SemaphoreHandle_t mutex;

// number of tasks to create
static const int num_tasks = 5;

// example struct for passing a string as a parameter
typedef struct Message {
  char body[20];
  uint8_t len;
} Message;

void myTask(void *parameters) {
  // copy the message struct from the parameter to a local variable
  Message msg = *(Message *)parameters;

  // serial is now protected by mutex
  Serial.print("Received:\t");
  Serial.print(msg.body);
  Serial.print("\t|\tlen:\t");
  Serial.println(msg.len);

  xSemaphoreGive(mutex);
  // increment semaphore to indicate that the parameter has been read
  xSemaphoreGive(count_sem);

  // wait for a while and delete self
  vTaskDelay(1000 / portTICK_PERIOD_MS);
  vTaskDelete(NULL);
}

void setup() {
  char task_name[12];
  Message msg;
  char text[20] = "All your base";

  Serial.begin(115200);
  Serial.println("---FreeRTOS Counting Semaphore Demo---");

  // create semaphores (initialize at 0)
  count_sem = xSemaphoreCreateCounting(num_tasks, 0);
  mutex = xSemaphoreCreateMutex();

  strcpy(msg.body, text);
  msg.len = strlen(text);

  for (int i = 0; i < num_tasks; i++) {
    sprintf(task_name, "Task %i", i);
    xSemaphoreTake(mutex, portMAX_DELAY);
    xTaskCreatePinnedToCore(myTask, task_name, 1024, (void *)&msg, 1, NULL, app_cpu);
  }

  // wait for all tasks to read shared memory
  for (int i = 0; i < num_tasks; i++) {
    xSemaphoreTake(count_sem, portMAX_DELAY);
  }

  // notify that all tasks have been created
  Serial.println("All tasks created!");
}

void loop() {
}
