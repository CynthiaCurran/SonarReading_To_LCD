/**********************************************************************
  Filename    : Display Ultrasonic Readings to LCD Display
  Description : I2C is used to control the display characters of LCD1602.
  Auther      : Cynthia Curran
  Modification: 2024/09/10
**********************************************************************/
#include <LiquidCrystal_I2C.h>
#include <Wire.h>
#include <stdio.h>
#include <string.h>

// Use only core 1 for demo purposes
#if CONFIG_FREERTOS_UNICORE
  static const BaseType_t app_cpu = 0;
#else
  static const BaseType_t app_cpu = 1;
#endif

//LCD Pins
#define SDA 13                    //Define SDA pins
#define SCL 14                    //Define SCL pins
LiquidCrystal_I2C lcd(0x27,16,2); 

//Ultrasonic Pins
#define trigPin 15 // define TrigPin
#define echoPin 18 // define EchoPin.
#define MAX_DISTANCE 700 // Maximum sensor distance is rated at 400-500cm.
//timeOut= 2*MAX_DISTANCE /100 /340 *1000000 = MAX_DISTANCE*58.8
float timeOut = MAX_DISTANCE * 60; 
int soundVelocity = 340; // define sound speed=340m/s
float sonarDistance = 0;


//TASKS

void readSonar(void *parameter){

  while(1){
    unsigned long pingTime;
    float distance;
    // make trigPin output high level lasting for 10μs to triger HC_SR04
    digitalWrite(trigPin, HIGH); 
    delayMicroseconds(10);
    digitalWrite(trigPin, LOW);
    // Wait HC-SR04 returning to the high level and measure out this waitting time
    pingTime = pulseIn(echoPin, HIGH, timeOut); 
    // calculate the distance according to the time
    distance = (float)pingTime * soundVelocity / 2 / 10000; 
    sonarDistance = distance; // return the distance value

    vTaskDelay(100 / portTICK_PERIOD_MS);
  }
}

void displayLCD(void *parameter){
  LiquidCrystal_I2C *lcd = (LiquidCrystal_I2C *) parameter;
  char buffer[8];

  while(1){
    sprintf(buffer, "%.0f", sonarDistance);
    int length = strlen(buffer);
    sprintf(buffer, "%.0f%*s", sonarDistance, 8 - length, " ");
    lcd->setCursor(0,1);
    lcd->print(buffer);
    vTaskDelay(500 / portTICK_PERIOD_MS);
  }
}


void setup() {

  pinMode(trigPin,OUTPUT);// set trigPin to output mode
  pinMode(echoPin,INPUT); // set echoPin to input mode

  Wire.begin(SDA, SCL);           // attach the IIC pin
  if (!i2CAddrTest(0x27)) {
    lcd = LiquidCrystal_I2C(0x3F, 16, 2);
  }
  lcd.init();                     // LCD driver initialization
  lcd.backlight();                // Open the backlight
  lcd.setCursor(0,0);             // Move the cursor to row 0, column 0
  lcd.print("cm away");     // The print content is displayed on the LCD

    // Start sonar reading task
  xTaskCreatePinnedToCore(  // Use xTaskCreate() in vanilla FreeRTOS
            readSonar,      // Function to be called
            "Read Sonar",   // Name of task
            2048,           // Stack size (bytes in ESP32, words in FreeRTOS)
            NULL,           // Parameter to pass
            1,              // Task priority
            NULL,           // Task handle
            app_cpu);       // Run on one core for demo purposes (ESP32 only)

      // Start lcd display task
  xTaskCreatePinnedToCore(  // Use xTaskCreate() in vanilla FreeRTOS
            displayLCD,      // Function to be called
            "Display to LCD",   // Name of task
            2048,           // Stack size (bytes in ESP32, words in FreeRTOS)
            (void*)&lcd,           // Parameter to pass
            2,              // Task priority
            NULL,           // Task handle
            app_cpu);       // Run on one core for demo purposes (ESP32 only)

  // Delete "setup and loop" task
  vTaskDelete(NULL);
}

void loop() {
  // put your main code here, to run repeatedly:

}

bool i2CAddrTest(uint8_t addr) {
  Wire.beginTransmission(addr);
  if (Wire.endTransmission() == 0) {
    return true;
  }
  return false;
}
