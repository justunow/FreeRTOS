/* FreeRTOS mutex */

#if CONFIG_FREERTOS_UNICORE
static const BaseType_t app_cpu = 0;
#else
static const BaseType_t app_cpu = 1;
#endif

static SemaphoreHandle_t mutex;
static const int led = LED_BUILTIN;

void blinkLED(void *parameters) {
  // copy the parameter into a local variable
  int num = *(int *)parameters;

  xSemaphoreGive(mutex);

  // print the parameter
  Serial.print("Received:\t");
  Serial.println(num);

  pinMode(led, OUTPUT);

  while (1) {
    digitalWrite(led, HIGH);
    vTaskDelay(num / portTICK_PERIOD_MS);
    digitalWrite(led, LOW);
    vTaskDelay(num / portTICK_PERIOD_MS);
  }
}

void setup() {
  // warning from FreeRTOS: do not pass arguments that on local stack memory 
  long int delay_arg;

  Serial.begin(115200);
  while (Serial.available() <= 0);
  delay_arg = Serial.parseInt();
  Serial.print("Sending:\t");
  Serial.println(delay_arg);

  // mutex initializes at 1
  mutex = xSemaphoreCreateMutex();

  xSemaphoreTake(mutex, portMAX_DELAY);

  xTaskCreatePinnedToCore(blinkLED, "Blink LED", 1024, (void *)&delay_arg, 1, NULL, app_cpu);
  // without delay the setup task will go out of scope first,
  // then the value passed to the blink task will not exist
  // vTaskDelay(1000 / portTICK_PERIOD_MS);

  // wait for task to give back the mutex without a timeout
  xSemaphoreTake(mutex, portMAX_DELAY);

  // show that we accomplished our task of passing the stack-based argument
  Serial.println("Done!");
}

void loop() {
}
