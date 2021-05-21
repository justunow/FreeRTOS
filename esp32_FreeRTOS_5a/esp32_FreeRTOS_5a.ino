/* FreeRTOS queue demo */

#if CONFIG_FREERTOS_UNICORE
static const BaseType_t app_cpu = 0;
#else
static const BaseType_t app_cpu = 1;
#endif

static QueueHandle_t msg_queue;
static const uint8_t msg_queue_len = 5;

/* task: wait for item on queue and print it */
void printMessages(void *parameters) {
  // queue item will be copied to local variable
  int item;
  while (1) {
    // see if there is a message in the queue (do not block)
    if (xQueueReceive(msg_queue, (void *)&item, 0) == pdTRUE) {
      Serial.println(item);
    }
    // Serial.println(item);
    vTaskDelay(100 / portTICK_PERIOD_MS);
  }
}

/* task: send item to the queue */
void sendMessages(void *parameter) {
  // if this variable is defined in while function, need to be declared as static
  int num = 0;
  while (1) {
    // try to add item to queue for 10 ticks, fail if queue is full
    if (xQueueSend(msg_queue, (void *)&num, 10) != pdTRUE) {
      Serial.println("Queue full");
    }
    else {
      num++;
    }
    vTaskDelay(500 / portTICK_PERIOD_MS);
  }
}

void setup() {
  Serial.begin(115200);
  Serial.println("---FreeRTOS Queue Demo---");

  // create queue, queue length (5 is maximum), size of each item
  msg_queue = xQueueCreate(msg_queue_len, sizeof(int));

  xTaskCreatePinnedToCore(printMessages, "Print Messages", 1024, NULL, 1, NULL, app_cpu);
  xTaskCreatePinnedToCore(sendMessages, "Send Messages", 1024, NULL, 1, NULL, app_cpu);

  vTaskDelete(NULL);
}

void loop() {
}
