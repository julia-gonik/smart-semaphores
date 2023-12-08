#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/semphr.h>
#include <stdio.h>

#define NUM_SEMAFOROS 4

SemaphoreHandle_t mutex;
QueueHandle_t queue;

int autos[] = {0, 0, 0, 0};
int alreadyInQueue[] = {0, 0, 0, 0};

void agregarAutoEsperando(int esquina, int cantidad = 1)
{
  Serial.print("Agregando ");
  Serial.print(cantidad);
  Serial.print(" autos a la esquina ");
  Serial.println(esquina);
  autos[esquina] += cantidad;
}

void prenderLedVerde(int esquina)
{
  Serial.print("Luz verde en ");
  Serial.print(esquina);
  Serial.print(". Autos: ");
  Serial.println(autos[esquina]);
}

void prenderLedRojo(int esquina)
{
  Serial.print("Luz roja en ");
  Serial.print(esquina);
  Serial.print(". Autos: ");
  Serial.println(autos[esquina]);
}

void eliminarAutoEsperando(int esquina)
{
  autos[esquina]--;
}

void vTask(void *arg)
{
  int id = *((int *)arg);

  while (1)

    if (autos[id] != 0)
    {
      if (!alreadyInQueue[id])
      {

        if (xSemaphoreTake(mutex, portMAX_DELAY) == pdTRUE)
        {
          xQueueSendToBack(queue, &id, (TickType_t)10);
          alreadyInQueue[id] = 1;
          xSemaphoreGive(mutex);
        }
      }
      else

      {
        if (xSemaphoreTake(mutex, portMAX_DELAY) == pdTRUE)
        {
          int info;

          while (info != id)
          {
            xQueuePeek(queue, &info, (TickType_t)10);
            xSemaphoreGive(mutex);
            xSemaphoreTake(mutex, portMAX_DELAY);
          }

          int received;
          xQueueReceive(queue, &received, (TickType_t)10);
          alreadyInQueue[id] = 0;

          prenderLedVerde(id);

          while (autos[id] > 0)
          {
            delay(100);
            Serial.print("Eliminando auto de ");
            Serial.println(id);
            autos[id]--;
          }

          prenderLedRojo(id);

          xSemaphoreGive(mutex);
        }
      }
    }
  vTaskDelay(100 / portTICK_PERIOD_MS);
}

void addCarsTask1(void *arg)
{
  while (1)
  {
    // Simulate the addition of cars
    agregarAutoEsperando(1, 5);

    vTaskDelay(1500 / portTICK_PERIOD_MS); // Delay for 5 seconds
  }
}

void addCarsTask2(void *arg)
{
  while (1)
  {
    // Simulate the addition of cars
    agregarAutoEsperando(2, 2);

    vTaskDelay(3000 / portTICK_PERIOD_MS); // Delay for 5 seconds
  }
}

void addCarsTask3(void *arg)
{
  while (1)
  {
    // Simulate the addition of cars
    agregarAutoEsperando(3, 1);

    vTaskDelay(2000 / portTICK_PERIOD_MS); // Delay for 5 seconds
  }
}

void addCarsTask0(void *arg)
{
  while (1)
  {
    // Simulate the addition of cars
    agregarAutoEsperando(0, 10);

    vTaskDelay(5000 / portTICK_PERIOD_MS); // Delay for 5 seconds
  }
}

void setup()
{
  Serial.begin(115200);

  mutex = xSemaphoreCreateMutex();
  queue = xQueueCreate(4, sizeof(int));

  for (int i = 0; i < NUM_SEMAFOROS; i++)
  {
    agregarAutoEsperando(i, 3);
  }

  for (int i = 0; i < 4; i++)
  {

    int *taskId = new int(i);
    xTaskCreate(vTask, "Semaforo", 2048, (void *)taskId, 1, NULL);
  }

  xTaskCreate(addCarsTask0, "AddCarsTask", 2048, NULL, 1, NULL);
  xTaskCreate(addCarsTask1, "AddCarsTask", 2048, NULL, 1, NULL);
  xTaskCreate(addCarsTask2, "AddCarsTask", 2048, NULL, 1, NULL);
  xTaskCreate(addCarsTask3, "AddCarsTask", 2048, NULL, 1, NULL);
}

void loop()
{
}
