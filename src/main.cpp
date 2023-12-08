#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/semphr.h>
#include <stdio.h>

#define NUM_SEMAFOROS 4

SemaphoreHandle_t mutex;
QueueHandle_t queue;

int autos[] = {0, 0, 0, 0};

void agregarAutoEsperando(int esquina, int cantidad = 1)
{
  autos[esquina] += cantidad;
}

void eliminarAutoEsperando(int esquina)
{
  autos[esquina]--;
}

void vTask(void *arg)
{
  static int alreadyInQueue = 0;
  int id = *((int *)arg);
  int received;
  Serial.print("Entrando en id ");
  Serial.println(id);

  while (1)
  {
    if (autos[id] != 0 && !alreadyInQueue)
    {
      Serial.print("Holaaa");

      if (xSemaphoreTake(mutex, portMAX_DELAY) == pdTRUE)
      {
        Serial.print("chauu");
        xQueueSendToBack(queue, &id, (TickType_t)10);
        alreadyInQueue = 1;
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
        xQueueReceive(queue, &received, (TickType_t)10);
        alreadyInQueue = 0;
        while (autos[id] > 0)
        {
          eliminarAutoEsperando(id);
          Serial.print("Eliminando de id ");
          Serial.println(id);
        }

        xSemaphoreGive(mutex);
      }
    }
    vTaskDelay(100 / portTICK_PERIOD_MS);
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

  // Crear tareas para controlar semáforos
  for (int i = 0; i < 4; i++)
  {
    /*     Serial.print("creando el id ");
        Serial.print(i); */

    // Create a separate variable for each task
    int *taskId = new int(i);

    // Pass the address of the separate variable to the task
    xTaskCreate(vTask, "Semaforo", 2048, (void *)taskId, 1, NULL);
  }
}

void loop()
{
  // put your main code here, to run repeatedly:
}
