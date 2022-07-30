#include "NimBLEDevice.h"
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h> //https://github.com/adafruit/Adafruit_SSD1306

#define SERVICE        "ffc0"
#define CHAR1          "ffc1"
#define CHAR2          "ffc2"

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
// The pins for I2C are defined by the Wire-library. 
// On an arduino UNO:       A4(SDA), A5(SCL)
// On an arduino MEGA 2560: 20(SDA), 21(SCL)
// On an arduino LEONARDO:   2(SDA),  3(SCL), ...
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);



BLEScan *pScan;
BLEScanResults pResults;

BLEClient *pClient;
BLERemoteService *pService;
BLERemoteCharacteristic *pChar1, *pChar2;
BLEAdvertisedDevice device;

BLEUUID ServiceUuid(SERVICE);


class MyClientCallback : public BLEClientCallbacks
{
  void onConnect(BLEClient *pclient)
  {
    Serial.println("Callback: connected");
  display.clearDisplay();
  display.setTextSize(2);             // Normal 1:1 pixel scale
  display.setTextColor(SSD1306_WHITE);        // Draw white text
  display.setCursor(31,0);             // Start at top-left corner
  display.println(F("LOLIN"));
  display.setCursor(12,25); 
  display.print("Connected");   
  display.display();
  }
  void onDisconnect(BLEClient *pclient)
  {
    Serial.println("Callback: disconnected");   
  display.clearDisplay();
  display.setTextSize(2);             // Normal 1:1 pixel scale
  display.setTextColor(SSD1306_WHITE);        // Draw white text
  display.setCursor(31,0);             // Start at top-left corner
  display.println(F("LOLIN"));
  display.setCursor(12,25); 
  display.print("Disconnected");   
  display.display();
  }
};

void notifyCB(BLERemoteCharacteristic* pRemoteCharacteristic, uint8_t* pData, size_t length, bool isNotify) {
  int i;
  Serial.print("From Spark: ");
  for (i = 0; i < length; i++) {
    Serial.print(pData[i], HEX);
  }
  Serial.println();
}

void connect() {
  bool found = false;
  bool connected = false;
  int i;
  
  BLEDevice::init("SparkLOLIN");
  pClient = BLEDevice::createClient();
  pClient->setClientCallbacks(new MyClientCallback());
 
  pScan = BLEDevice::getScan();

  while (!found) {
    pResults = pScan->start(4);
    
    for(i = 0; i < pResults.getCount()  && !found; i++) {
      device = pResults.getDevice(i);

      if (device.isAdvertisingService(ServiceUuid)) {
        Serial.println("Found Mini");
       display.clearDisplay();
  display.setTextSize(2);             // Normal 1:1 pixel scale
  display.setTextColor(SSD1306_WHITE);        // Draw white text
  display.setCursor(31,0);             // Start at top-left corner
  display.println(F("LOLIN"));
  display.setCursor(0,25); 
  display.print("FoundMini");   
  display.display();
        found = true;
        connected = false;
      }
    }

    // found it, now connect
   
    if (pClient->connect(&device)) {
      connected = true;
      pService = pClient->getService(ServiceUuid);
      if (pService != nullptr) {
        pChar1   = pService->getCharacteristic(CHAR1);
        pChar2   = pService->getCharacteristic(CHAR2);        
        if (pChar2 && pChar2->canNotify()) {
          pChar2->registerForNotify(notifyCB);
          if (!pChar2->subscribe(true, notifyCB, true)) {
            connected = false;
            Serial.println("Disconnected");
            display.clearDisplay();
            display.setTextSize(2);             // Normal 1:1 pixel scale
            display.setTextColor(SSD1306_WHITE);        // Draw white text
            display.setCursor(31,0);             // Start at top-left corner
            display.println(F("LOLIN"));
            display.setCursor(0,25); 
            display.print("Disconnected");
            display.display();
            NimBLEDevice::deleteClient(pClient);
          }   
        } 
      }
    }
  }
}

unsigned long t1, t2;
int preset;
int new_preset;

const uint8_t switchPins[]{32,33,25,26};

void setup() {
  Wire.begin(0,4);
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  Serial.begin(115200);

  for (int i=0; i < 4; i++)
    pinMode(switchPins[i], INPUT_PULLUP); 
  
  Serial.println("Connecting...");
  display.clearDisplay();
  display.setTextSize(2);             // Normal 1:1 pixel scale
  display.setTextColor(SSD1306_WHITE);        // Draw white text
  display.setCursor(31,0);             // Start at top-left corner
  display.println(F("LOLIN"));
  display.setCursor(0,25); 
  display.print("Connecting");   
  display.display();
  connect();
  Serial.println("Connected");
    display.clearDisplay();
  display.setTextSize(2);             // Normal 1:1 pixel scale
  display.setTextColor(SSD1306_WHITE);        // Draw white text
  display.setCursor(31,0);             // Start at top-left corner
  display.println(F("LOLIN"));
  display.setCursor(0,25); 
  display.print("Connected");   
  display.display();
  t1 = t2 = millis();
  preset = 0;
  new_preset = 0;
}

byte preset_cmd[] = {
  0x01, 0xFE, 0x00, 0x00,
  0x53, 0xFE, 0x1A, 0x00,
  0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00,
  0xF0, 0x01, 0x24, 0x00,
  0x01, 0x38, 0x00, 0x00,
  0x00, 0xF7
};
const int preset_cmd_size = 26;

void loop() {
  if (millis() - t1 > 5000) {
    t1 = millis();
    // time process
//    new_preset++;    
//    if (new_preset > 3) new_preset = 0;
  }

  if (millis() - t2 > 100) {
    t2 = millis();
    // GPIO process      
    for (int i = 0; i < 4; i++) {
      if (digitalRead(switchPins[i]) == 0) {
        new_preset = i;
        Serial.print("Got a switch ");
        Serial.println(i);
      }
    }
  }
   
  if (new_preset != preset) {    
    preset = new_preset;
    preset_cmd[preset_cmd_size-2] = preset;
    Serial.print("Sent a preset change to ");
    Serial.println(preset);
    display.clearDisplay();
    display.setTextSize(2);             // Normal 1:1 pixel scale
    display.setTextColor(SSD1306_WHITE);        // Draw white text
    display.setCursor(31,0);             // Start at top-left corner
    display.println(F("PRESET"));
    display.setCursor(50,25); 
    display.setTextSize(4);
    display.print(preset +1);   
    display.display();
    pChar1->writeValue(preset_cmd, preset_cmd_size);
  }
}
