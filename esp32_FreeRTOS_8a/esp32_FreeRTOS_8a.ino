/* FreeRTOS software timer demo */

#if CONFIG_FREERTOS_UNICORE
static const BaseType_t app_cpu = 0;
#else
static const BaseType_t app_cpu = 1;
#endif

static TimerHandle_t one_shot_timer = NULL;
static TimerHandle_t auto_reload_timer = NULL;

void myTimerCallback(TimerHandle_t xTimer) {
  if ((uint32_t)pvTimerGetTimerID(xTimer) == 0) {
    Serial.println("One-shot timer expired");
  }
  if ((uint32_t)pvTimerGetTimerID(xTimer) == 1) {
    Serial.println("Auto-reload timer expired");
  }
}

void setup() {
  Serial.begin(115200);
  Serial.println("---FreeRTOS Timer Demo---");

  // create a one-shot timer
  one_shot_timer = xTimerCreate(
                     "One-shot timer",           // name of timer
                     2000 / portTICK_PERIOD_MS,  // period of timer (int ticks)
                     pdFALSE,                    // auto-reload
                     (void *)0,                  // timer ID
                     myTimerCallback);           // callback function

  // create an auto-reload timer
  auto_reload_timer = xTimerCreate(
                        "Auto-reload timer",        // name of timer
                        1000 / portTICK_PERIOD_MS,  // period of timer (int ticks)
                        pdTRUE,                     // auto-reload
                        (void *)1,                  // timer ID
                        myTimerCallback);           // callback function

  if (one_shot_timer == NULL || auto_reload_timer == NULL) {
    Serial.println("Could not create one of the timers");
  }
  else {
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    Serial.println("Starting timers...");

    // start timers (max block time if command queue is full)
    xTimerStart(one_shot_timer, portMAX_DELAY);
    xTimerStart(auto_reload_timer, portMAX_DELAY);
  }

  vTaskDelete(NULL);
}

void loop() {
}
