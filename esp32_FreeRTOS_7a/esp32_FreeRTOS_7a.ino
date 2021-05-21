/* FreeRTOS semaphore demo - binary semaphore */

#if CONFIG_FREERTOS_UNICORE
static const BaseType_t app_cpu = 0;
#else
static const BaseType_t app_cpu = 1;
#endif

static SemaphoreHandle_t bin_sem;
static const int led = LED_BUILTIN;

void blinkLED(void *parameters) {
  // copy the parameter into a local variable
  int num = *(int *)parameters;

  // release the binary semaphore so that the creating function can finish
  xSemaphoreGive(bin_sem);

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
  long int delay_arg;

  Serial.begin(115200);
  while (Serial.available() <= 0);
  delay_arg = Serial.parseInt();
  Serial.print("Sending:\t");
  Serial.println(delay_arg);

  // binary semaphore initializes at 0, no need to take before the creating function
  bin_sem = xSemaphoreCreateBinary();

  xTaskCreatePinnedToCore(blinkLED, "Blink LED", 1024, (void *)&delay_arg, 1, NULL, app_cpu);

  // do nothing until binary semaphore has been returned
  xSemaphoreTake(bin_sem, portMAX_DELAY);

  // show that we accomplished our task of passing the stack-based argument
  Serial.println("Done!");
}

void loop() {
}
