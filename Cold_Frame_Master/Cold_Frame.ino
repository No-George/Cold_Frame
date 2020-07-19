//Set board as esp32 wrover w/huge app

// Replace with your unique IFTTT URL resource
const char* server = "maker.ifttt.com";
const char* resource = "place your IFTTT link";

// Replace with your wifi settings
const char* ssid     = "ssid";
const char* password = "password";

//Mosquitto settings
const char* mqtt_server = "mosquitto IP";
//#define mqtt_user "username"     
//#define mqtt_password "password"   


//Pinmap
#define SDA 16
#define SCL 14
#define senseEN 12
#define dallasPin 13 
#define LED 4

#include "Variables.h"

void setup(void) 
{
   Serial.begin(115200);
    if (cameraFlag==1){
      cameraFlag = 0;
      snaptime = timestamp;
      captureImage(); 
    }
    else{     
       sensorInit();
       Serial.println("Sensors Initialized");
       updateADC();
       Serial.println("ADC updated");
       updateDallas();
       Serial.println("Dallas updated");
       updateBME();
       Serial.println("BME updated");
       sensorReport();  
       initWifi();
       //initNTP();   //use if not running mosquitto 
       publishMQTT();
       MQTTdateTime();
       makeIFTTTRequest();
    }

    sleepPlan();
   ++bootCount;
   timestamp = timestamp + TIME_TO_SLEEP + millis()/1000; //increment timestamp even when network fails so it is still about right
   Serial.print("bootCount "); Serial.println(bootCount);
   Serial.print("exe time "); Serial.println(millis());
   Serial.print("going to sleep now for "); Serial.print(TIME_TO_SLEEP);Serial.println(" seconds");
   esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
   esp_deep_sleep_start();
}

void loop(void) 
{

}

void sensorReport(){
  Serial.print("solarVoltage: "); Serial.println(solarVoltage);
  Serial.print("batteryVoltage: "); Serial.println(batteryVoltage);
  Serial.print("moistureReading: "); Serial.println(moistureReading);
  Serial.print("soilTemp ");    Serial.println(soilTemp);
  Serial.print("outsideTemp ");  Serial.println(outsideTemp);
  Serial.print("frameTemp = ");Serial.print(frameTemp);    Serial.println(" *C");
  Serial.print("Pressure = ");     Serial.print(pressure);    Serial.println(" kPa");
  Serial.print("Humidity = ");    Serial.print(humidity);    Serial.println(" %");
  Serial.println();
}

void sleepPlan(){
  if (((timestamp-snaptime)>3600)&&(batteryVoltage>3.5)&&(hour(timestamp)>10)&&(hour(timestamp)<20)){cameraFlag=1;TIME_TO_SLEEP=1;}
  else if (batteryVoltage>4){TIME_TO_SLEEP=900;}
  else if (batteryVoltage>3.6){TIME_TO_SLEEP=1800;}
  else if (batteryVoltage>3.4){TIME_TO_SLEEP=3600;}
  else {TIME_TO_SLEEP=7200;}

    //Forces LED properly Off
  pinMode(4, OUTPUT);
  digitalWrite(4, LOW);    //power down the flash
  pinMode(32, OUTPUT);
  digitalWrite(32, LOW); //power down the camera
  rtc_gpio_hold_en(GPIO_NUM_4);
  rtc_gpio_hold_en(GPIO_NUM_32);
  
}
