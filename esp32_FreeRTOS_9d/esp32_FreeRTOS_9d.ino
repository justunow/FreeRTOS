/* FreeRTOS hardware interrupts */

/*
  #if CONFIG_FREERTOS_UNICORE
  static const BaseType_t app_cpu = 0;
  #else
  static const BaseType_t app_cpu = 1;
  #endif
 */

static const BaseType_t pro_cpu = 0;
static const BaseType_t app_cpu = 1;

static const char command[] = "avg";
static const uint16_t timer_divider = 8;
static const uint64_t timer_max_count = 1000000;
static const uint32_t cli_delay = 20;
enum { BUF_LEN = 10 };
enum { MSG_LEN = 100 };
enum { MSG_QUEUE_LEN = 5 };
enum { CMD_BUF_LEN = 255};

static const int adc_pin = A0;

// message struct to wrap strings for queue
typedef struct Message {
  char body[MSG_LEN];
} Message;

static hw_timer_t *timer = NULL;
static TaskHandle_t processing_task = NULL;
static SemaphoreHandle_t sem_done_reading = NULL;
static portMUX_TYPE spinlock = portMUX_INITIALIZER_UNLOCKED;
static QueueHandle_t msg_queue;
static volatile uint16_t buf_0[BUF_LEN];
static volatile uint16_t buf_1[BUF_LEN];
static volatile uint16_t* write_to = buf_0;   // double buffer write pointer
static volatile uint16_t* read_from = buf_1;  // double buffer read pointer
static volatile uint8_t buf_overrun = 0;      // double buffer overrun flag
static float adc_avg;

// swap the write_to and read_from pointers in the double buffer
// only ISR calls this at the moment, so no need to make it thread-safe
void IRAM_ATTR swap() {
  volatile uint16_t* temp_ptr = write_to;
  write_to = read_from;
  read_from = temp_ptr;
}

void IRAM_ATTR onTimer() {
  static uint16_t idx = 0;
  BaseType_t task_woken = pdFALSE;

  // if buffer is not overrun, read ADC to next buffer element. If buffer is
  // overrun, drop the sample.
  if ((idx < BUF_LEN) && (buf_overrun == 0)) {
    write_to[idx] = analogRead(adc_pin);
    idx++;
  }

  // check if the buffer is full
  if (idx >= BUF_LEN) {
    if (xSemaphoreTakeFromISR(sem_done_reading, &task_woken) == pdFALSE) {
      buf_overrun = 1;
    }

    // only swap buffers and notify task if overrun flag is cleared
    if (buf_overrun == 0) {
      // reset index and swap buffer pointers
      idx = 0;
      swap();

      // a task notification works like a binary semaphore but is faster
      vTaskNotifyGiveFromISR(processing_task, &task_woken);
    }
  }

  if (task_woken) {
    portYIELD_FROM_ISR();
  }
}

// serial terminal task
void doCLI(void *parameters) {
  Message rcv_msg;
  char c;
  char cmd_buf[CMD_BUF_LEN];
  uint8_t idx = 0;
  uint8_t cmd_len = strlen(command);

  // clear whole buffer
  memset(cmd_buf, 0, CMD_BUF_LEN);

  while (1) {
    // look for any error messages that need to be printed
    if (xQueueReceive(msg_queue, (void *)&rcv_msg, 0) == pdTRUE) {
      Serial.println(rcv_msg.body);
    }
    if (Serial.available() > 0) {
      c = Serial.read();
      if (idx < CMD_BUF_LEN - 1) {
        cmd_buf[idx] = c;
        idx++;
      }
      if ((c == '\n') || (c == '\r')) {
        Serial.print("\r\n");
        cmd_buf[idx - 1] = '\0';
        if (strcmp(cmd_buf, command) == 0) {
          Serial.print("Average:\t");
          Serial.println(adc_avg);
        }

        memset(cmd_buf, 0, CMD_BUF_LEN);
        idx = 0;
      } else {
        Serial.print(c);
      }
    }

    vTaskDelay(cli_delay / portTICK_PERIOD_MS);
  }
}

// wait for semaphore and calculate average of ADC values
void calcAverage(void *parameters) {
  Message msg;
  float avg;

  timer = timerBegin(0, timer_divider, true);
  timerAttachInterrupt(timer, &onTimer, true);
  timerAlarmWrite(timer, timer_max_count, true);
  timerAlarmEnable(timer);

  while (1) {
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

    avg = 0.0;
    for (int i = 0; i < BUF_LEN; i++) {
      avg += (float)read_from[i];
      //vTaskDelay(105 / portTICK_PERIOD_MS); // uncomment to test overrun flag
    }
    avg /= BUF_LEN;

    portENTER_CRITICAL(&spinlock);
    adc_avg = avg;
    portEXIT_CRITICAL(&spinlock);

    if (buf_overrun == 1) {
      strcpy(msg.body, "Error: Buffer overrun. Samples have been dropped.");
      xQueueSend(msg_queue, (void *)&msg, 10);
    }

    portENTER_CRITICAL(&spinlock);
    buf_overrun = 0;
    xSemaphoreGive(sem_done_reading);
    portEXIT_CRITICAL(&spinlock);
  }
}

void setup() {
  Serial.begin(115200);

  sem_done_reading = xSemaphoreCreateBinary();
  if (sem_done_reading == NULL) {
    Serial.println("Could not create one or more semaphores");
    ESP.restart();
  }

  xSemaphoreGive(sem_done_reading);

  msg_queue = xQueueCreate(MSG_QUEUE_LEN, sizeof(Message));

  xTaskCreatePinnedToCore(doCLI, "Do CLI", 1024, NULL, 2, NULL, app_cpu);
  xTaskCreatePinnedToCore(calcAverage, "Calculate average", 1024, NULL, 1, &processing_task, pro_cpu);

  /* setup() and loop() functions run in a task that is priority 1 pinned to core 1,
     move the initialization functions to a task that runs on core 0, so we can start
     the hardware timer and run ISR in core 0
    timer = timerBegin(0, timer_divider, true);
    timerAttachInterrupt(timer, &onTimer, true);
    timerAlarmWrite(timer, timer_max_count, true);
    timerAlarmEnable(timer);
   */

  vTaskDelete(NULL);
}

void loop() {
}
