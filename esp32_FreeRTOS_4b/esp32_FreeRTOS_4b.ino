/* FreeRTOS stack overflow */

#if CONFIG_FREERTOS_UNICORE
static const BaseType_t app_cpu = 0;
#else
static const BaseType_t app_cpu = 1;
#endif

static volatile bool flag = false;
static char *ptr = NULL;
static const uint8_t buf_len = 255;

/* task 1: read from monitor and store into heap */
void readSerial(void *parameter) {
  char buf[buf_len];
  uint8_t idx = 0;

  memset(buf, 0, buf_len);
  while (1) {
    if (Serial.available()) {
      if (idx < buf_len - 1) {
        buf[idx] = Serial.read();
        idx++;
      }
      if (buf[idx - 1] == '\n') {
        buf[idx - 1] = '\0';

        if (flag == false) {
          Serial.print("Heap before malloc (bytes):\t");
          Serial.println(xPortGetFreeHeapSize());
          
          ptr = (char *)pvPortMalloc(idx * sizeof(char));
          
          // if malloc returns 0 (out of memory), throw an error and reset
          configASSERT(ptr);
          memcpy(ptr, buf, idx);
          flag = true;
        }
        memset(buf, 0, buf_len);
        idx = 0;
      }
    }
  }
}

/* task 2: read out from heap and print to monitor */
void printMessage(void *parameter) {
  while (1) {
    if (flag == true) {
      Serial.println(ptr);

      Serial.print("Heap after malloc (bytes):\t");
      Serial.println(xPortGetFreeHeapSize());

      vPortFree(ptr);
      ptr = NULL;
      flag = false;
    }
  }
}

void setup() {
  Serial.begin(115200);

  xTaskCreatePinnedToCore(readSerial, "Read Serial", 1024, NULL, 1, NULL, app_cpu);
  xTaskCreatePinnedToCore(printMessage, "Print Message", 1024, NULL, 1, NULL, app_cpu);

  vTaskDelete(NULL);
}

void loop() {
}
