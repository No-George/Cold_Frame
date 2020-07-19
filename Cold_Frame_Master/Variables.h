//Cam
#include "esp_camera.h"
#include "Arduino.h"
#include "FS.h"                // SD Card ESP32
#include "SD_MMC.h"            // SD Card ESP32
#include "soc/soc.h"           // Disable brownout problems
#include "soc/rtc_cntl_reg.h"  // Disable brownout problems

#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27

#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22
RTC_DATA_ATTR bool cameraFlag = 0;

//RTC
#include <time.h>
#include "driver/rtc_io.h"
long exeTime = 0;
int TIME_TO_SLEEP = 30;        /*900 = 15 mins Time ESP32 will go to sleep (in seconds) */

//Wifi
#include <WiFi.h>
#include <TimeLib.h>
#include <WiFiUdp.h>

  const int timeZone = 0;     // GMT
  static const char ntpServerName[] = "time.nist.gov";
  WiFiUDP Udp;
  unsigned int localPort = 8888;  // local port to listen for UDP packets
  time_t getNtpTime();
  time_t NTP = 0;
  void sendNTPpacket(IPAddress &address);

//Sensors
#include <Wire.h>
#include <Adafruit_ADS1015.h>
#include <Adafruit_BME280.h>
#include <DallasTemperature.h>
#include <OneWire.h>

  Adafruit_ADS1115 ads;  /* Use this for the 16-bit version */
  Adafruit_BME280 bme;
  OneWire oneWire(dallasPin); 
  DallasTemperature dallas(&oneWire);
  unsigned BMEstatus; 

//RTC vars
//gpio_num_t pin_FAN = GPIO_NUM_15;
RTC_DATA_ATTR int bootCount = 0;
RTC_DATA_ATTR time_t timestamp = 0;
RTC_DATA_ATTR time_t snaptime = 0;
#define uS_TO_S_FACTOR 1000000ULL  /* Conversion factor for micro seconds to seconds */



//Dallas
RTC_DATA_ATTR float soilTemp = -127;
RTC_DATA_ATTR float outsideTemp = -127;

//BME
 float frameTemp = 0;
 float humidity = 0;
 float pressure = 0;
 
//ADC 
 int moistureReading = 0;
 RTC_DATA_ATTR float batteryVoltage = 0;
 float solarVoltage = 0;

 //Network
const int NTP_PACKET_SIZE = 48; // NTP time is in the first 48 bytes of message
byte packetBuffer[NTP_PACKET_SIZE]; //buffer to hold incoming & outgoing packets

//MQTT
//#define MQTT_MAX_PACKET_SIZE 100000
#include <PubSubClient.h>


WiFiClient coldFrame;
PubSubClient client(coldFrame);



#define MQTTsoilTemp "Cold_Frame/Soil_Temp"       
#define MQTTframeTemp "Cold_Frame/Frame_Temp"       
#define MQTToutsideTemp "Cold_Frame/Outside_Temp"       
#define MQTTsolarVoltage "Cold_Frame/Solar_Voltage"       
#define MQTTbatteryVoltage "Cold_Frame/Battery_Voltage"       
#define MQTTmoistureReading "Cold_Frame/Moisture_Reading"       
#define MQTTpressure "Cold_Frame/Pressure"       
#define MQTThumidity "Cold_Frame/Humidity"       
#define MQTTimage "Cold_Frame/Image"   
bool response = 0;
