/* FreeRTOS stack overflow demo */

#if CONFIG_FREERTOS_UNICORE
static const BaseType_t app_cpu = 0;
#else
static const BaseType_t app_cpu = 1;
#endif

void testTask(void *parameter) {
  while (1) {
    int a = 1, b[100];

    // do something with array so it is not optimized out by the compiler
    for (int i = 0; i < 100; i++) {
      b[i] = a + i;
    }
    Serial.println(b[0]);

    // print out remaining stack memory (words)
    Serial.print("High water mark (words):\t");
    Serial.println(uxTaskGetStackHighWaterMark(NULL));

    // print out number of free heap memory bytes before malloc
    Serial.print("Heap before malloc (bytes):\t");
    Serial.println(xPortGetFreeHeapSize());
    int *ptr = (int *)pvPortMalloc(1024 * sizeof(int));

    // one way to prevent heap overflow is to check the malloc output
    if (ptr == NULL) {
      Serial.println("Not enough heap!");
    }
    else {
      // do something with the memory so it is not optimized out by the compiler
      for (int i = 0; i < 1024; i++) {
        ptr[i] = 2;
      }
    }

    // print out number of free heap memory bytes after malloc
    Serial.print("Heap after malloc (bytes):\t");
    Serial.println(xPortGetFreeHeapSize());

    // free up allocated memory
    vPortFree(ptr);

    vTaskDelay(100 / portTICK_PERIOD_MS);
  }
}

void setup() {
  Serial.begin(115200);
  Serial.println("---FreeRTOS Memory Demo---");

  // 1024 bytes < minimum stack requirement 768 bytes + sizeof(int) * 100
  xTaskCreatePinnedToCore(testTask, "Test Task", 1500, NULL, 1, NULL, app_cpu);

  vTaskDelete(NULL);
}

void loop() {
}
