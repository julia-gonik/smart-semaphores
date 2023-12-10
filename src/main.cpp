#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/semphr.h>
#include <stdio.h>

#define FLASH_GPIO_NUM 4
#define NUM_SEMAFOROS 4
#define QUANTUM 2

void salida(int numSem);
void apagar();
void prender(int semNuevo);
void cadena();
void limpieza();

SemaphoreHandle_t mutex;
QueueHandle_t queue;

int autos[] = {0, 0, 0, 0};
int alreadyInQueue[] = {0, 0, 0, 0};
int ledPin = 12;  // SERIAL DATA INPUT - DS
int mostrar = 13; // este se usa para sacar los datos
int reloj = 15;   // SHCP
                  // sem1 ,sem2 ,sem3 ,sem4
                // r,a,v,r,a,v,r,a,v,r,a,v
int valores[12] = {1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0};
int semAct = 5; // semaforo prendido en verde en el momento
// si es 0 pos --> r 0 a 1 v 2
// si es 1 pos --> r 3 a 4 v 5
// si es 2 pos --> r 6 a 7 v 8
// si es 3 pos --> r 9 a 10 v 11

void salida(int numSem)
{
  if (semAct != numSem)
  {
    // apagar();
    prender(numSem);
    semAct = numSem;
  }
}
void apagar()
{
  if (semAct < 4)
  {
    //(semaforo*3)+color
    int pos = 0;
    // apagar verde
    pos = semAct * 3 + 2;
    valores[pos] = 0;
    // prender amarillo
    pos = semAct * 3 + 1;
    valores[pos] = 1;
    cadena();
    delay(2000);
    // apagar amarillo
    pos = semAct * 3 + 1;
    valores[pos] = 0;
    // prender rojo
    pos = semAct * 3 + 0;
    valores[pos] = 1;
    cadena();
    delay(2000);
  }
}
void prender(int semNuevo)
{
  //(semaforo*3)+color
  int pos = 0;
  // apagar rojo
  pos = semNuevo * 3 + 0;
  valores[pos] = 0;
  // prender amarillo
  pos = semNuevo * 3 + 1;
  valores[pos] = 1;
  cadena();
  delay(2000);
  // apagar amarillo
  pos = semNuevo * 3 + 1;
  valores[pos] = 0;
  // prender verde
  pos = semNuevo * 3 + 2;
  valores[pos] = 1;
  cadena();
  delay(2000);
}
void cadena()
{
  int tam = 12;
  delay(50);
  // Recorrer el array de atrás hacia adelante con un bucle for inverso
  for (int i = tam - 1; i >= 0; --i)
  {
    digitalWrite(reloj, LOW);
    digitalWrite(ledPin, valores[i]); // le pongo el valor el LED
    digitalWrite(reloj, HIGH);
    digitalWrite(ledPin, 0); // le pongo el valor el LED
  }
  digitalWrite(reloj, LOW);
  digitalWrite(mostrar, LOW);
  delay(1);
  digitalWrite(mostrar, HIGH);
  delay(1);
}

void limpieza()
{
  // primero limpio los registros
  digitalWrite(ledPin, LOW); // APAGA el LED
  // primero limpio los registros
  for (int i = 0; i < 16; i++)
  {
    digitalWrite(reloj, HIGH);
    delay(50);
    digitalWrite(reloj, LOW);
    delay(50);
  }
  digitalWrite(mostrar, LOW);
  delay(50);
  digitalWrite(mostrar, HIGH);
  delay(50);
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
  salida(esquina);
}

void prenderLedRojo(int esquina)
{
  Serial.print("Luz roja en ");
  Serial.println(esquina);
  apagar();
}

void eliminarAutoEsperando(int esquina)
{
  autos[esquina]--;
}

void vTask(void *arg)
{
  int id = *((int *)arg);
  int quantum = QUANTUM;

  while (1)
  {
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

          while (autos[id] > 0 && quantum > 0)
          {
            delay(50);
            Serial.print("Eliminando de id ");
            Serial.println(id);
            autos[id]--;
            quantum--;
          }
          if (autos[id] > 0)
          {
            xQueueSendToBack(queue, &id, (TickType_t)10);
          }

          if (quantum == 0)
          {
            quantum = QUANTUM;
          }

          prenderLedRojo(id);

          xSemaphoreGive(mutex);
        }
      }
    }
    vTaskDelay(100 / portTICK_PERIOD_MS);
  }
}

void addCarsTask1(void *arg)
{
  while (1)
  {
    // Simulate the addition of cars
    agregarAutoEsperando(1, 5);

    vTaskDelay(4000 / portTICK_PERIOD_MS); // Delay for 5 seconds
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

  pinMode(FLASH_GPIO_NUM, OUTPUT);
  pinMode(ledPin, OUTPUT);  // Configura el pin del LED como salida
  pinMode(mostrar, OUTPUT); // Configura el pin del LED como salida
  pinMode(reloj, OUTPUT);   // Configura el pin del LED como salida
  delay(50);

  digitalWrite(reloj, LOW); // colocamos en  0 para dsp ponerlo en 1 y generar el flanco de subida
  digitalWrite(mostrar, LOW);

  limpieza();
  valores[1] = 1;
  cadena();
  vTaskDelay(100 / portTICK_PERIOD_MS);

  digitalWrite(FLASH_GPIO_NUM, HIGH);
  delay(200);
  digitalWrite(FLASH_GPIO_NUM, LOW);

  for (int i = 0; i < NUM_SEMAFOROS; i++)
  {
    agregarAutoEsperando(i, 3);
  }

  for (int i = 0; i < 4; i++)
  {

    int *taskId = new int(i);
    xTaskCreate(vTask, "Semaforo", 2048, (void *)taskId, 1, NULL);
  }

  xTaskCreate(addCarsTask0, "AddCarsTask", 1024, NULL, 3, NULL);
  xTaskCreate(addCarsTask1, "AddCarsTask", 1024, NULL, 3, NULL);
  xTaskCreate(addCarsTask2, "AddCarsTask", 1024, NULL, 3, NULL);
  xTaskCreate(addCarsTask3, "AddCarsTask", 1024, NULL, 3, NULL);

  vTaskStartScheduler();
}

void loop()
{
}