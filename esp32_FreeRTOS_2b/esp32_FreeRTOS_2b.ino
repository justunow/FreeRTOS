/* FreeRTOS LED blinks at different rates */

#if CONFIG_FREERTOS_UNICORE
static const BaseType_t app_cpu = 0;
#else
static const BaseType_t app_cpu = 1;
#endif

static const int led_pin = LED_BUILTIN;

void toggleLED(void *parameter) {
  while (1) {
    digitalWrite(led_pin, HIGH);
    vTaskDelay(500 / portTICK_PERIOD_MS);
    digitalWrite(led_pin, LOW);
    vTaskDelay(500 / portTICK_PERIOD_MS);
  }
}

void toggleLED1(void *parameter) {
  while (1) {
    digitalWrite(led_pin, HIGH);
    vTaskDelay(200 / portTICK_PERIOD_MS);
    digitalWrite(led_pin, LOW);
    vTaskDelay(200 / portTICK_PERIOD_MS);
  }
}

void setup() {
  pinMode(led_pin, OUTPUT);

  xTaskCreatePinnedToCore(  // use xTaskCreate() in vanilla FreeRTOS
    toggleLED,    // function to be called
    "Toggle LED", // name of task
    1024,         // stack size (bytes in ESP32, words in FreeRTOS)
    NULL,         // parameter to pass to function
    1,            // task priority (0 to configMAX_PRIORITIES - 1)
    NULL,         // task handle
    app_cpu);     // run on one core for demo purposes (ESP32 only)

  xTaskCreatePinnedToCore(  // use xTaskCreate() in vanilla FreeRTOS
    toggleLED1,   // function to be called
    "Toggle LED1",// name of task
    1024,         // stack size (bytes in ESP32, words in FreeRTOS)
    NULL,         // parameter to pass to function
    2,            // task priority (0 to configMAX_PRIORITIES - 1)
    NULL,         // task handle
    app_cpu);     // run on one core for demo purposes (ESP32 only)
}

void loop() {
}
