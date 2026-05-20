/******************************************************************************************
 * Button Signal zu TouchDesigner
 * LED leuchtet gelb, wenn der angeschlossene Hardware-Button am ESP32 gedrückt wird
 * Install library "OSC" by Adrian Freed
 * specify your Wifi ssid and pw, and IP address of your receiver PC
 ******************************************************************************************/


#include <WiFi.h>
#include <WiFiUdp.h>
#include <OSCMessage.h>
#include <OSCBundle.h>
#include <OSCData.h>

const char* ssid = "dreammakers";   // @todo: add your wifi name
const char* pass = "dreammakers";   // @todo: add your wifi pw
bool isWlanConnected = 0;

WiFiUDP Udp;                                 // A UDP instance to let us send and receive packets over UDP
const IPAddress remoteIp(192, 168, 0, 116);  // @todo: add receiver IP address
const unsigned int remotePort = 9000;        
const unsigned int localPort = 8000;        

const int buttonPin = 7;                 
int buttonState = 0;         
int prev_buttonState = 0;
const int led =  RGB_BUILTIN;                        

void setup() {
  Serial.begin(115200);
  connectWiFi();
  connectUdp();

  pinMode(buttonPin, INPUT_PULLDOWN);  
  pinMode(led, OUTPUT);    
  rgbLedWrite(led, 0, 255, 0); // grb rot
}

void loop() {
  if (!is_wlan_connected()) return; 
  sendOSC();    
}

void connectWiFi(){
    Serial.printf("\nVerbinde mit WLAN %s", ssid); // ssid ist const char*, kein String(ssid) nötig
    WiFi.begin(ssid, pass);

    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 40)
    {                                    // Max 20 Versuche (10 Sekunden)
        delay(500);
        Serial.print(".");
        attempts++;
    }
    if (WiFi.status() == WL_CONNECTED){
        Serial.printf("\nWiFi verbunden: SSID: %s, IP-Adresse: %s\n", ssid, WiFi.localIP().toString().c_str());
        rgbLedWrite(led, 255, 0, 0);     // GRB: grün
    }
    else{
        Serial.println("\n❌ WiFi Verbindung fehlgeschlagen!");
        rgbLedWrite(led, 0, 255, 0);     // GRB: rot
    }
}

void connectUdp() {
  Udp.begin(localPort);
  Serial.println("Starting UDP - Local port: " + String(localPort));
}


void sendOSC() {
  ////////////////// get button value
  buttonState = digitalRead(buttonPin);
  if(buttonState == prev_buttonState) return;
  prev_buttonState = buttonState;

  ////////////////// feedback on serial port abd LED
  if (buttonState == 1) rgbLedWrite(led, 255, 255, 0);  // LED gelb
  else digitalWrite(led, 0);                            // LED aus
  Serial.println("/btn1: " + String(buttonState));

  ////////////////// send value via OSC
  OSCMessage msg("/btn1");        // define OSC key
  msg.add((int32_t)buttonState);   // define OSC value
  Udp.beginPacket(remoteIp, remotePort);
  msg.send(Udp);
  Udp.endPacket();
  msg.empty();
}


bool is_wlan_connected(){
  if (WiFi.status() != WL_CONNECTED) {
    if (isWlanConnected == 1) {          // War vorher verbunden?
      Serial.println("WiFi-Verbindung verloren, reconnect...");
      rgbLedWrite(led, 0, 255, 0);       // GRB: Rot
      isWlanConnected = 0;
    }
    connectWiFi(); 
    return false; // Loop wird abgebrochen
  }
  return true; // WiFi ist da, Loop darf weiterlaufen
}