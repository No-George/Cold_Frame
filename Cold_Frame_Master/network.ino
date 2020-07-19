
void initWifi() {
  Serial.print("Connecting to: "); 
  Serial.print(ssid);
  WiFi.begin(ssid, password);  

  int timeout = 10 * 4; // 10 seconds
  while(WiFi.status() != WL_CONNECTED  && (timeout-- > 0)) {
    delay(250);
    Serial.print(".");
  }
  Serial.println("");

  if(WiFi.status() != WL_CONNECTED) {
     Serial.println("Failed to connect, going back to sleep");
  }

  Serial.print("WiFi connected in: "); 
  Serial.print(millis());
  Serial.print(", IP address: "); 
  Serial.println(WiFi.localIP());
}

void initNTP(){
  //Serial.println("Starting UDP");
  Udp.begin(localPort);
  //Serial.print("Local port: ");
  //Serial.println(Udp.localPort());
  //Serial.println("waiting for sync");
  setSyncProvider(getNtpTime);
  setSyncInterval(300);
  if (timeStatus() != timeNotSet) {
      NTP = now();
    }
  }

time_t getNtpTime(){
  IPAddress ntpServerIP; // NTP server's ip address

  while (Udp.parsePacket() > 0) ; // discard any previously received packets
  Serial.println("Transmit NTP Request");
  // get a random server from the pool
  WiFi.hostByName(ntpServerName, ntpServerIP);
  //Serial.print(ntpServerName);
  //Serial.print(": ");
  //Serial.println(ntpServerIP);
  sendNTPpacket(ntpServerIP);
  uint32_t beginWait = millis();
  while (millis() - beginWait < 1500) {
    int size = Udp.parsePacket();
    if (size >= NTP_PACKET_SIZE) {
      Serial.println("Receive NTP Response");
      Udp.read(packetBuffer, NTP_PACKET_SIZE);  // read packet into the buffer
      unsigned long secsSince1900;
      // convert four bytes starting at location 40 to a long integer
      secsSince1900 =  (unsigned long)packetBuffer[40] << 24;
      secsSince1900 |= (unsigned long)packetBuffer[41] << 16;
      secsSince1900 |= (unsigned long)packetBuffer[42] << 8;
      secsSince1900 |= (unsigned long)packetBuffer[43];
      return secsSince1900 - 2208988800UL + timeZone * SECS_PER_HOUR;
    }
  }
  Serial.println("No NTP Response :-(");
  return 0; // return 0 if unable to get the time
}

  void sendNTPpacket(IPAddress &address){
  // set all bytes in the buffer to 0
  memset(packetBuffer, 0, NTP_PACKET_SIZE);
  // Initialize values needed to form NTP request
  // (see URL above for details on the packets)
  packetBuffer[0] = 0b11100011;   // LI, Version, Mode
  packetBuffer[1] = 0;     // Stratum, or type of clock
  packetBuffer[2] = 6;     // Polling Interval
  packetBuffer[3] = 0xEC;  // Peer Clock Precision
  // 8 bytes of zero for Root Delay & Root Dispersion
  packetBuffer[12] = 49;
  packetBuffer[13] = 0x4E;
  packetBuffer[14] = 49;
  packetBuffer[15] = 52;
  // all NTP fields have been given values, now
  // you can send a packet requesting a timestamp:
  Udp.beginPacket(address, 123); //NTP requests are to port 123
  Udp.write(packetBuffer, NTP_PACKET_SIZE);
  Udp.endPacket();
}

// Make an HTTP request to the IFTTT web service
void makeIFTTTRequest() {
  Serial.print("Connecting to ");   Serial.print(server);
  
  WiFiClient client;
  int retries = 5;
  while(!!!client.connect(server, 80) && (retries-- > 0)) {
    Serial.print(".");
  }
  Serial.println();
  if(!!!client.connected()) {
    Serial.println("Failed to connect...");
  }
  
  Serial.print("Request resource: "); 
  Serial.println(resource);

  exeTime = millis();
  // Temperature in Celsius
  String jsonObject = String("{\"value1\":\"") + snaptime + " ||| " + exeTime
                      + " ||| " + solarVoltage + " ||| " + batteryVoltage + " ||| " + moistureReading + " ||| " + soilTemp + " ||| " + outsideTemp + " ||| " + frameTemp + " ||| " + pressure + " ||| " + humidity + "\"}";
                                          
  client.println(String("POST ") + resource + " HTTP/1.1");
  client.println(String("Host: ") + server); 
  client.println("Connection: close\r\nContent-Type: application/json");
  client.print("Content-Length: ");
  client.println(jsonObject.length());
  client.println();
  client.println(jsonObject);
        
  int timeout = 5 * 10; // 5 seconds             
  while(!!!client.available() && (timeout-- > 0)){
    delay(100);
  }
  if(!!!client.available()) {
    Serial.println("No response...");
  }
  while(client.available()){
    Serial.write(client.read());
  }
  
  Serial.println("\nclosing connection");
  client.stop(); 
}

//Reconnect to MQTT if connection is lost
void reconnect() {
  client.setServer(mqtt_server, 1883);    // Configure MQTT connection, change port if needed.
  client.setCallback(callback);
  int retries = 5;
  while (!client.connected()&& (retries-- > 0)) {
    Serial.print("Connecting to MQTT broker ...");
    if (client.connect("coldFrame")) {//, mqtt_user, mqtt_password)) {
      Serial.println("OK");
      client.subscribe("dateTime");
    } else {
      Serial.print("[Error] Not connected: ");
      Serial.print(client.state());
      Serial.println("Wait 5 seconds before retry.");
      delay(1000);
    }
  }
}

void publishMQTT(){
  
  if (!client.connected()) {  //limited to afew attempts
    reconnect();
  }
    // Publish values to MQTT topics
    client.publish(MQTTsoilTemp, String(soilTemp).c_str(), true);   // Publish temperature on broker/temp1
    client.publish(MQTTframeTemp, String(frameTemp).c_str(), true);   // Publish temperature on broker/temp1
    client.publish(MQTToutsideTemp, String(outsideTemp).c_str(), true);   // Publish temperature on broker/temp1
    client.publish(MQTTmoistureReading, String(moistureReading).c_str(), true);
    client.publish(MQTTpressure, String(pressure).c_str(), true);
    client.publish(MQTThumidity, String(humidity).c_str(), true);
    client.publish(MQTTsolarVoltage, String(solarVoltage).c_str(), true);   // Publish temperature on broker/temp1
    client.publish(MQTTbatteryVoltage, String(batteryVoltage).c_str(), true);   // Publish temperature on broker/temp1
    Serial.println("publishing MQTT");
    if(client.endPublish ()){
      Serial.println("Success");
    }else{Serial.println("MQTT Failed");}
    Serial.println("\nclosing connection");
    coldFrame.stop(); 
}



void MQTTdateTime(){
    while((millis()<10000) && (response == 0)){
        if (!client.connected()) {
          reconnect();
      }else {    
        client.loop();    
      }
    }
}

void callback(char* topic, byte* payload, unsigned int length) {
  char message_buff[50];
  Serial.print("Message arrived [");  Serial.print(topic);
  Serial.print("] ");
  int i = 0;
  for (i = 0; i < length; i++) {
    message_buff[i] = payload[i];
    //Serial.print(message_buff[i]);
  }
  Serial.println();
  message_buff[i-3] = '\0'; ///crop to seconds
  const char *p_payload = message_buff;
  //if (strcmp(topic,dateTime) == 0){Cupboard_Temp = atof(p_payload);response=1;}
  if (strcmp(topic,"dateTime") == 0){timestamp = atol(p_payload);response=1;}
  Serial.print("timestamp "); Serial.println(timestamp);
}
