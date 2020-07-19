 
void sensorInit(){
  ///enable power
   pinMode(senseEN, OUTPUT);
   digitalWrite(senseEN,HIGH);
   delay(50);
   ///enable I2C for BME280 and ADC
   Wire.begin(SDA, SCL); 

   ///setup BME
   BMEstatus = bme.begin(0x76, &Wire);

   ///setupdallas
   dallas.begin();
   
}

void updateADC(){
  //adc0 = ads.readADC_SingleEnded(0);
  solarVoltage = 0.0003750*ads.readADC_SingleEnded(1);//2*1875/1000 voltage divider &volts
  if (solarVoltage>6){solarVoltage=0;}
  batteryVoltage = 0.0003750*ads.readADC_SingleEnded(2);
  moistureReading = ads.readADC_SingleEnded(3);
}

void updateDallas(){
  int attempts = 0;
  int T0 = -127;
  int T1 = -127;
  do{
  dallas.requestTemperatures();
  T0 = dallas.getTempCByIndex(1);
  T1 = dallas.getTempCByIndex(0);
  if (T0>-100){soilTemp = T0;}
  if (T1>-100){outsideTemp = T1;}
  attempts++;
  }while((T0+T1)<-40 && attempts<40);  ///If either sensor doesnt work for some reason then try again
}


void updateBME(){
  if (BMEstatus){    
    frameTemp = bme.readTemperature();
    pressure = bme.readPressure() / 1000.0F;    
    humidity = bme.readHumidity();    
  }
}
