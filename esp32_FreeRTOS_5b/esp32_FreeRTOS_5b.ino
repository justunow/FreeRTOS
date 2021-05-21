/* FreeRTOS queue */

#if CONFIG_FREERTOS_UNICORE
static const BaseType_t app_cpu = 0;
#else
static const BaseType_t app_cpu = 1;
#endif

static QueueHandle_t queueOne;
static QueueHandle_t queueTwo;
static const uint8_t queue_len = 5;
static const int led = LED_BUILTIN;
unsigned long blinkPeriod = 1000;
unsigned long count = 0;

typedef struct Message {
  char body[20];
  int count;
} Message;

/* task A: prints messages from queue 2, reads serial input and echo
   to serial monitor, if "delay xxx", sends xxx to queue 1 */
void messages(void *parameters) {
  Message msg;
  String incomingBytes;
  char subString[20];
  while (1) {
    if (xQueueReceive(queueTwo, (void *)&msg, 0) == pdTRUE) {
      Serial.print(msg.body);
      Serial.print(msg.count);
      Serial.println("\ttimes");
    }
    if (Serial.available()) {
      incomingBytes = Serial.readStringUntil('\n');
      Serial.println(incomingBytes);
      if (incomingBytes.substring(0, 5) == "delay") {
        incomingBytes.substring(6).toCharArray(subString, 20);
        blinkPeriod = atoi(subString);
        if (xQueueSend(queueOne, (void *)&blinkPeriod, 10) != pdTRUE) {
          Serial.println("Queue full");
        }
      }
    }
  }
}

/* task B: updates led blink rate, sends "blinked xxx times" every 100 times */
void blinkLed(void *parameters) {
  Message sendMsg;
  while (1) {
    xQueueReceive(queueOne, (void *)&blinkPeriod, 0);

    digitalWrite(led, HIGH);
    vTaskDelay(blinkPeriod / portTICK_PERIOD_MS);
    digitalWrite(led, LOW);
    vTaskDelay(blinkPeriod / portTICK_PERIOD_MS);

    count++;
    if (count % 100 == 0) {
      strcpy(sendMsg.body, "Blinked:\t");
      sendMsg.count = count;
      if (xQueueSend(queueTwo, (void *)&sendMsg, 10) != pdTRUE) {
        Serial.println("Queue full");
      }
    }
  }
}

void setup() {
  pinMode(led, OUTPUT);
  Serial.begin(115200);

  queueOne = xQueueCreate(queue_len, sizeof(int));
  queueTwo = xQueueCreate(queue_len, sizeof(Message));

  xTaskCreatePinnedToCore(messages, "Messages", 1024, NULL, 1, NULL, app_cpu);
  xTaskCreatePinnedToCore(blinkLed, "Blink Led", 1024, NULL, 1, NULL, app_cpu);

  vTaskDelete(NULL);
}

void loop() {
}
