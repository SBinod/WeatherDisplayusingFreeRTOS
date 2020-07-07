// LiquidCrystal - Version: Latest 
#include <LiquidCrystal.h>

#include "FreeRTOS.h"
#include "queue.h"
#include <DHT.h>

#define DHTPIN 2     // what pin we're connected to
#define DHTTYPE DHT22   // DHT 22
DHT dht(DHTPIN, DHTTYPE); // Initialize DHT sensor

typedef struct
{
  float temperature;
  float humidity;
}Data_t;

QueueHandle_t queue_1; //Handle of the queue created

void setup() 
{
  Serial.begin(9600);
  queue_1 = xQueueCreate(3, sizeof(Data_t));
  if (queue_1 == NULL) 
  {
    Serial.println("No space for data to be logged in"); //As queue could not be created
  }
  xTaskCreate(lcd_task, "LCD TASK", 200, NULL, 1, NULL);
  xTaskCreate(temp_task, "TEMPERATURE TASK", 200, NULL, 1, NULL);
  vTaskStartScheduler(); //starts the scheduler
}

void temp_task(void *pvParameters) 
{
  BaseType_t xStatus;
  const TickType_t xTickstoWait = pdMS_TO_TICKS(500);
  Data_t datatosend;

  while(1) 
  {
    datatosend.temperature = dht.readTemperature();
    datatosend.humidity = dht.readHumidity();
    xStatus = xQueueSend(queue_1, &datatosend, xTickstoWait);
    if(xStatus != pdPASS)
    {
      Serial.println("The data could not be sent to the Queue...");
    }
    else
    {
      Serial.print("Sent the data to the queue");
    }
    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}

void lcd_task(void *pvParameters) 
{
  BaseType_t xStatus;
  Data_t datatoreceive;
  while(1) 
  {
    LiquidCrystal lcd(12, 11, 5, 4, 3, 2); // Pins RST E D4 D5 D6 D7
    lcd.begin(16, 2);
    xStatus = xQueueReceive(queue_1, &datatoreceive, portMAX_DELAY);
    
    if(xStatus == pdPASS)
    {
      lcd.setCursor(0, 0);
      lcd.print("TEMPERATURE(C): ");
      lcd.print(datatoreceive.temperature);
      lcd.setCursor(0, 1);
      lcd.print("HUMIDITY(%): ");
      lcd.print(datatoreceive.humidity);
    }
    else
    {
      lcd.setCursor(0,0); 
      lcd.print("Waiting for data");
    }
  }
}

void loop(){}
