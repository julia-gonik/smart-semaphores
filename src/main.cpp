#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/semphr.h>
#include <stdio.h>

#define NUM_SEMAFOROS 4

SemaphoreHandle_t autos_mutex;
SemaphoreHandle_t semaforos[] = {0, 0, 0, 0};
SemaphoreHandle_t scheduler;

int autos[] = {0, 0, 0, 0};

void agregarAutoEsperando(int esquina, int cantidad = 1)
{
  Serial.print("Agregando en ");
  Serial.print(esquina);
  Serial.print(". Autos: ");
  Serial.println(cantidad);
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
  Serial.println(esquina);
}

void eliminarAutoEsperando(int esquina)
{
  autos[esquina]--;
}

int encontrarIndiceNumeroMasChico(int *tieneAutos)
{

  int indiceMenor = 0;

  for (int i = 0; i < NUM_SEMAFOROS; i++)
  {
    if (autos[i] != 0 && autos[i] < autos[indiceMenor])
    {
      // Encontramos un número más pequeño, actualizamos el índice
      indiceMenor = i;
    }
  }
  *tieneAutos = autos[indiceMenor] != 0;
  return indiceMenor;
}
void vTaskScheduler(void *arg)
{
  int siguienteSemaforo = -1;
  int tieneAutos = 0;

  while (1)
  {
    if (xSemaphoreTake(scheduler, portMAX_DELAY) == pdTRUE)
    {
      siguienteSemaforo = encontrarIndiceNumeroMasChico(&tieneAutos);

      if (tieneAutos)
      {
        xSemaphoreGive(semaforos[siguienteSemaforo]);
      }
    }

    vTaskDelay(100 / portTICK_PERIOD_MS);
  }
}

void vTask(void *arg)
{
  int id = *((int *)arg);

  while (1)
  {
    if (xSemaphoreTake(semaforos[id], portMAX_DELAY) == pdTRUE)
    {

      prenderLedVerde(id);

      if (xSemaphoreTake(autos_mutex, portMAX_DELAY) == pdTRUE)
      {
        while (autos[id] > 0)
        {
          delay(50);
          Serial.print("Eliminando de id ");
          Serial.println(id);
          autos[id]--;
        }
      }

      prenderLedRojo(id);

      xSemaphoreGive(autos_mutex);
      xSemaphoreGive(scheduler);
    }
    vTaskDelay(100 / portTICK_PERIOD_MS);
  }
}

void addCarsTask1(void *arg)
{
  while (1)
  {
    if (xSemaphoreTake(autos_mutex, portMAX_DELAY) == pdTRUE)
    {
      agregarAutoEsperando(1, 5);
    }

    xSemaphoreGive(autos_mutex);
    vTaskDelay(4000 / portTICK_PERIOD_MS); // Delay for 5 seconds
  }
}

void addCarsTask2(void *arg)
{
  while (1)
  {
    if (xSemaphoreTake(autos_mutex, portMAX_DELAY) == pdTRUE)
    {
      agregarAutoEsperando(2, 2);
    }

    xSemaphoreGive(autos_mutex);
    vTaskDelay(3000 / portTICK_PERIOD_MS); // Delay for 5 seconds
  }
}

void addCarsTask3(void *arg)
{
  while (1)
  {
    if (xSemaphoreTake(autos_mutex, portMAX_DELAY) == pdTRUE)
    {
      agregarAutoEsperando(3, 1);
    }

    xSemaphoreGive(autos_mutex);
    vTaskDelay(2000 / portTICK_PERIOD_MS); // Delay for 5 seconds
  }
}

void addCarsTask0(void *arg)
{
  while (1)
  {
    if (xSemaphoreTake(autos_mutex, portMAX_DELAY) == pdTRUE)
    {
      agregarAutoEsperando(0, 10);
    }

    xSemaphoreGive(autos_mutex);
    vTaskDelay(5000 / portTICK_PERIOD_MS); // Delay for 5 seconds
  }
}

void setup()
{
  Serial.begin(115200);

  autos_mutex = xSemaphoreCreateMutex();
  semaforos[0] = xSemaphoreCreateBinary();
  semaforos[1] = xSemaphoreCreateBinary();
  semaforos[2] = xSemaphoreCreateBinary();
  semaforos[3] = xSemaphoreCreateBinary();
  scheduler = xSemaphoreCreateBinary();

  agregarAutoEsperando(0, 1);
  agregarAutoEsperando(1, 2);
  agregarAutoEsperando(2, 3);
  agregarAutoEsperando(3, 4);

  for (int i = 0; i < 4; i++)
  {
    int *taskId = new int(i);
    xTaskCreate(vTask, "Semaforo", 2048, (void *)taskId, 1, NULL);
  }

  xTaskCreate(addCarsTask0, "AddCarsTask", 1024, NULL, 3, NULL);
  xTaskCreate(addCarsTask1, "AddCarsTask", 1024, NULL, 3, NULL);
  xTaskCreate(addCarsTask2, "AddCarsTask", 1024, NULL, 3, NULL);
  xTaskCreate(addCarsTask3, "AddCarsTask", 1024, NULL, 3, NULL);

  xTaskCreate(vTaskScheduler, "scheduler", 2048, NULL, 1, NULL);

  xSemaphoreGive(scheduler);
}

void loop()
{
}
