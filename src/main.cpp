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
int estaUnSemaforoCorriendo = 0;

int getRandomInteger(int min, int max)
{
  // Seed the random number generator with the current time
  srand((unsigned int)time(NULL));

  // Generate a random number between min and max (inclusive)
  return min + rand() % (max - min + 1);
}

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
    Serial.print("Esquina ");
    Serial.print(i);
    Serial.print(" autos: ");
    Serial.println(autos[i]);
    if (autos[i] != 0 && autos[i] < autos[indiceMenor] || autos[indiceMenor] == 0)
    {
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
    if (!estaUnSemaforoCorriendo)
    {
      if (xSemaphoreTake(autos_mutex, portMAX_DELAY) == pdTRUE)
      {
        siguienteSemaforo = encontrarIndiceNumeroMasChico(&tieneAutos);

        if (tieneAutos)
        {
          estaUnSemaforoCorriendo = 1;
          xSemaphoreGive(semaforos[siguienteSemaforo]);
        }

        xSemaphoreGive(autos_mutex);
      }
    }
    vTaskDelay(2000 / portTICK_PERIOD_MS); // Delay for 5 seconds
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

        prenderLedRojo(id);

        xSemaphoreGive(autos_mutex);
        estaUnSemaforoCorriendo = 0;
      }
    }
  }
}

void addCarsTask(void *arg)
{
  while (1)
  {
    if (xSemaphoreTake(autos_mutex, portMAX_DELAY) == pdTRUE)
    {
      int random = getRandomInteger(1, 4);
      int esquina = getRandomInteger(0, NUM_SEMAFOROS - 1);
      agregarAutoEsperando(esquina, random);
    }

    xSemaphoreGive(autos_mutex);
    vTaskDelay(5000 / portTICK_PERIOD_MS); // Delay for 5 seconds
  }
}

void setup()
{
  Serial.begin(115200);

  autos_mutex = xSemaphoreCreateMutex();
  scheduler = xSemaphoreCreateBinary();

  for (int i = 0; i < NUM_SEMAFOROS; i++)
  {
    int *taskId = new int(i);

    semaforos[i] = xSemaphoreCreateBinary();
    agregarAutoEsperando(i, 1);
    xTaskCreate(vTask, "Semaforo", 2048, (void *)taskId, 1, NULL);
  }

  xTaskCreate(addCarsTask, "AddCarsTask", 1024, NULL, 3, NULL);
  xTaskCreate(vTaskScheduler, "scheduler", 2048, NULL, 1, NULL);

  xSemaphoreGive(scheduler);
}

void loop()
{
}
