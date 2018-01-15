
#include <ESP8266WiFi.h>        // Onboard WiFi and HTTP client
#include <Plantower_PMS7003.h>  // Particulate counter library
#include <SoftwareSerial.h>     // PMS7003 uses softserial on 13 (RX) and 15 (TX)

#include "config.h"


bool wifi_connected = false;
Plantower_PMS7003 pms7003 = Plantower_PMS7003();
SoftwareSerial pms7003_serial(13, 15);
char output[256];


void WiFiEvent(WiFiEvent_t event) {
  Serial.printf("[WiFi-event] event: %d\n", event);
  switch(event) {
    case WIFI_EVENT_STAMODE_GOT_IP:
      Serial.println("WiFi connected");
      Serial.println("IP address: ");
      Serial.println(WiFi.localIP());
      wifi_connected = true;
      break;
    case WIFI_EVENT_STAMODE_DISCONNECTED:
      Serial.println("WiFi lost connection");
      wifi_connected = false;
      break;
  }
}


void setup() {
  Serial.begin(115200);
  wifi_connected = false;

  // Init PMS7003 
  pms7003_serial.begin(9600);
  pms7003.init(&pms7003_serial);
  pms7003.debug = false;

  // Init Wifi
  WiFi.disconnect(true);
  WiFi.onEvent(WiFiEvent);
  delay(1000);
  WiFi.begin(WIFI_SSID, WIFI_PASS);

  Serial.println("\n\nSetup complete...");
}

void loop() {

  int updateReady = false;

  //
  // Process PMS7003 updates
  // 
  pms7003.updateFrame();
  if (pms7003.hasNewData()) {
    updateReady = true;
    sprintf(output, "\nSensor Version: %d    Error Code: %d\n",
                  pms7003.getHWVersion(),
                  pms7003.getErrorCode());
    Serial.print(output);

    sprintf(output, "    PM1.0 (ug/m3): %2d     [atmos: %d]\n",
                  pms7003.getPM_1_0(),
                  pms7003.getPM_1_0_atmos());              
    Serial.print(output);
    sprintf(output, "    PM2.5 (ug/m3): %2d     [atmos: %d]\n",
                  pms7003.getPM_2_5(),
                  pms7003.getPM_2_5_atmos());
    Serial.print(output);
    sprintf(output, "    PM10  (ug/m3): %2d     [atmos: %d]\n",
                  pms7003.getPM_10_0(),
                  pms7003.getPM_10_0_atmos());              
    Serial.print(output);

    sprintf(output, "\n    RAW: %2d[>0.3] %2d[>0.5] %2d[>1.0] %2d[>2.5] %2d[>5.0] %2d[>10]\n",
                  pms7003.getRawGreaterThan_0_3(),
                  pms7003.getRawGreaterThan_0_5(),
                  pms7003.getRawGreaterThan_1_0(),
                  pms7003.getRawGreaterThan_2_5(),
                  pms7003.getRawGreaterThan_5_0(),
                  pms7003.getRawGreaterThan_10_0());
    Serial.print(output);
  }


  //
  // Handle updates to server if WiFi connected.
  //
  
  if (!wifi_connected) {
    Serial.println("Waiting for WiFi connection...");
    delay(1000);
  } else if (updateReady) {
    Serial.print("WiFi RSSI: ");
    Serial.println(WiFi.RSSI());
    Serial.println("Have Wifi. Can send data here.");

    sendData(pms7003.getPM_1_0(),pms7003.getPM_2_5(),pms7003.getPM_10_0(), WiFi.RSSI());
    
  } else {
    // WiFi is up, but we don't have a full data frame yet.
  }

}


void sendData(int pm1_0, int pm2_5, int pm10_0, int rssi) {
  WiFiClient client;
      
  if (!client.connect(HTTP_SERVER, HTTP_PORT)) {
    Serial.println("Server connection failed");
    return;
  }

  String postdata = String("id=") + DEVICE_ID +
                    "&pm1_0=" + pm1_0 +
                    "&pm2_5=" + pm2_5 +
                    "&pm10_0=" + pm10_0 +
                    "&rssi=" + rssi;

  
  String post = String("POST ") + UPDATE_ENDPOINT + " HTTP/1.1\r\n" +
                "Host: " + HTTP_SERVER + "\r\n" + 
                "Connection: close\r\n" +
                "Content-Length: " + postdata.length() + "\r\n\r\n";
  Serial.print("SENDING:\n");
  Serial.print(post);
  Serial.println(postdata);
  
  client.print(post);
  client.print(postdata + "\r\n");

  unsigned long timeout = millis();
  while (client.available() == 0) {
    if (millis() - timeout > 5000) {
      Serial.println("ERROR: Timeout posting update");
      client.stop();
      return;
    }
  }

  // Read all the lines of the reply from server and print them to Serial
  while(client.available()){
    String line = client.readStringUntil('\r');
    Serial.print(line);
  }

  Serial.println();
  Serial.println("closing connection");
  client.stop();
}


