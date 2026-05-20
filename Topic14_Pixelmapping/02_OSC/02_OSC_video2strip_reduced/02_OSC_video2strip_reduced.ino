/*******************************************************************************
 * Empfange Lichtdaten per OSC von z. B. TouchDesigner (updated 18.05.2026)
 * LED-Ring zeigt Video, das in TouchDesigner für die LEDs portioniert wurde
 * Verbinde 12 WS2812B LEDs (z. B. LED-Ring) mit ESP32-C6:
 * WS2812B: Data in (Di)  <->  ESP32-C6: GPIO 2
 * WS2812B: 5V            <->  5V (Externe Stromversorgung, falls zu viele LEDs)
 * WS2812B: GND           <->  GND (Externe Stromversorgung, falls zu viele LEDs) 
 * installiere Libraries "OSC" by Adrian Freed und "Adafruit NeoPixel" by Adafruit
 * Ändere ssid, password, remote IP adress 
 ********************************************************************************/

#include <Adafruit_NeoPixel.h>

///////////////////////////////////////////////// WiFi & MQTT

#include <WiFi.h>
#include <WiFiUdp.h>
#include <OSCMessage.h>
// #include <OSCBundle.h>

const char* ssid = "tinkergarden";             // @todo: add your wifi name
const char* pass = "strenggeheim";             // @todo: add your wifi pw
bool isWlanConnected = 0;
int led = LED_BUILTIN;

WiFiUDP Udp;                                   
const unsigned int remotePort = 9000;          
const unsigned int localPort = 8000;        


////////////////////////////////////////////////// LED-Strip

const int stripPin = 2;      
const int num_leds = 12; 
Adafruit_NeoPixel strip(num_leds, stripPin, NEO_GRB + NEO_KHZ800);

void setup() {
  Serial.begin(115200);
  delay(1000);
  rgbLedWrite(led, 0, 255, 0);    // GRB rot
  connectWiFi();
  connectUdp();
  setupStrip();
}


void loop() {
  // Nur wenn die Verbindung steht, wird der Rest ausgeführt
  if (!is_wlan_connected()) {
    return; 
  }

  receiveOSC_StripData();   
}

///////////////////////////////////////////////// WiFi & UDP (OSC)

void connectWiFi() {
  WiFi.begin(ssid, pass);
  Serial.print("Connecting to WiFi...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.printf("WiFi connected: SSID: %s, IP Address: %s\n", ssid, WiFi.localIP().toString().c_str());
  rgbLedWrite(led, 255, 0, 0);  // Status: Rot
}


bool is_wlan_connected(){
  // 1. Fall: Überhaupt keine Hardware-Verbindung
  if (WiFi.status() != WL_CONNECTED) {
    if (isWlanConnected == 1) { // War vorher verbunden?
      Serial.println("WiFi-Verbindung verloren, reconnect...");
      rgbLedWrite(led, 0, 255, 0);  // Status: Rot
      isWlanConnected = 0;
    }
    connectWiFi(); 
    return false; // Loop wird abgebrochen
  }
  
  return true; // WiFi ist da, Loop darf weiterlaufen
}

void connectUdp() {
  Udp.begin(localPort);
  Serial.println("Starting UDP - Local port: " + String(localPort));
}

////////////////////////////////////////////////// LED-Strip

void setupStrip(){
  strip.begin();       
  strip.show();            
  strip.setBrightness(255); 
}

void receiveOSC_StripData(){
  int size = Udp.parsePacket();

  if (size > 0) {
    OSCMessage msg;
    while (size--) {
      msg.fill(Udp.read());
    }

    if (!msg.hasError()) {
      if (strcmp(msg.getAddress(), "/colors") == 0) {
        for(int i = 0; i < num_leds; i++) {
          strip.setPixelColor(i, msg.getInt(i*3), msg.getInt((i*3)+1), msg.getInt((i*3)+2));
        }
        strip.show();
      }
    }
  }
}