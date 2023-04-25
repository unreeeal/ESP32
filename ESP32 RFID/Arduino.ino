
// Feel free to use it if you subscribed to this channel: https://www.youtube.com/channel/UCWE70GK4SaYz1dxMMN0TzkQ
// The tutorial video is available here https://youtu.be/xeHBKi0GLow

//Tested with Arduino IDE 2.1.0
//Arduino core for the ESP32 2.0.7


//You need to install these libraries

//https://github.com/miguelbalboa/rfid
//https://gitlab.com/joearmstrong980/LCD_I2C


#include <SPI.h>
#include <MFRC522.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <driver/adc.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#define SS_PIN 15
#define RST_PIN 13
#define BUZZ_PIN 4
#define BUZZ_CHANNEL 2
#define DOOR_PIN 2
#define TERMINAL_NAME  "basement"

// SCRIPT LINK MUST BE LIKE THIS "https://script.google.com/macros/s/AKfycbzBCsPz3ZDVUkdP7mloaX1AAKI1mC_NxM802hvDNRwyE4vw4oo/exec"
const char *mainLinkForSpr = "";

const char *ssid = "wifi name";
const char *password = "password";
LiquidCrystal_I2C lcd(0x27, 16, 2);
MFRC522 mfrc522(SS_PIN, RST_PIN);

MFRC522::StatusCode status;

//#define DISABLE_WRITTING 
// #define OTA

uint64_t clearDisplayTimer=0;
bool needDisplayUpdate=true;

//  WiFiClientSecure client;
const char *root_ca =
"-----BEGIN CERTIFICATE-----\n" \
"MIIFVzCCAz+gAwIBAgINAgPlk28xsBNJiGuiFzANBgkqhkiG9w0BAQwFADBHMQsw\n" \
"CQYDVQQGEwJVUzEiMCAGA1UEChMZR29vZ2xlIFRydXN0IFNlcnZpY2VzIExMQzEU\n" \
"MBIGA1UEAxMLR1RTIFJvb3QgUjEwHhcNMTYwNjIyMDAwMDAwWhcNMzYwNjIyMDAw\n" \
"MDAwWjBHMQswCQYDVQQGEwJVUzEiMCAGA1UEChMZR29vZ2xlIFRydXN0IFNlcnZp\n" \
"Y2VzIExMQzEUMBIGA1UEAxMLR1RTIFJvb3QgUjEwggIiMA0GCSqGSIb3DQEBAQUA\n" \
"A4ICDwAwggIKAoICAQC2EQKLHuOhd5s73L+UPreVp0A8of2C+X0yBoJx9vaMf/vo\n" \
"27xqLpeXo4xL+Sv2sfnOhB2x+cWX3u+58qPpvBKJXqeqUqv4IyfLpLGcY9vXmX7w\n" \
"Cl7raKb0xlpHDU0QM+NOsROjyBhsS+z8CZDfnWQpJSMHobTSPS5g4M/SCYe7zUjw\n" \
"TcLCeoiKu7rPWRnWr4+wB7CeMfGCwcDfLqZtbBkOtdh+JhpFAz2weaSUKK0Pfybl\n" \
"qAj+lug8aJRT7oM6iCsVlgmy4HqMLnXWnOunVmSPlk9orj2XwoSPwLxAwAtcvfaH\n" \
"szVsrBhQf4TgTM2S0yDpM7xSma8ytSmzJSq0SPly4cpk9+aCEI3oncKKiPo4Zor8\n" \
"Y/kB+Xj9e1x3+naH+uzfsQ55lVe0vSbv1gHR6xYKu44LtcXFilWr06zqkUspzBmk\n" \
"MiVOKvFlRNACzqrOSbTqn3yDsEB750Orp2yjj32JgfpMpf/VjsPOS+C12LOORc92\n" \
"wO1AK/1TD7Cn1TsNsYqiA94xrcx36m97PtbfkSIS5r762DL8EGMUUXLeXdYWk70p\n" \
"aDPvOmbsB4om3xPXV2V4J95eSRQAogB/mqghtqmxlbCluQ0WEdrHbEg8QOB+DVrN\n" \
"VjzRlwW5y0vtOUucxD/SVRNuJLDWcfr0wbrM7Rv1/oFB2ACYPTrIrnqYNxgFlQID\n" \
"AQABo0IwQDAOBgNVHQ8BAf8EBAMCAYYwDwYDVR0TAQH/BAUwAwEB/zAdBgNVHQ4E\n" \
"FgQU5K8rJnEaK0gnhS9SZizv8IkTcT4wDQYJKoZIhvcNAQEMBQADggIBAJ+qQibb\n" \
"C5u+/x6Wki4+omVKapi6Ist9wTrYggoGxval3sBOh2Z5ofmmWJyq+bXmYOfg6LEe\n" \
"QkEzCzc9zolwFcq1JKjPa7XSQCGYzyI0zzvFIoTgxQ6KfF2I5DUkzps+GlQebtuy\n" \
"h6f88/qBVRRiClmpIgUxPoLW7ttXNLwzldMXG+gnoot7TiYaelpkttGsN/H9oPM4\n" \
"7HLwEXWdyzRSjeZ2axfG34arJ45JK3VmgRAhpuo+9K4l/3wV3s6MJT/KYnAK9y8J\n" \
"ZgfIPxz88NtFMN9iiMG1D53Dn0reWVlHxYciNuaCp+0KueIHoI17eko8cdLiA6Ef\n" \
"MgfdG+RCzgwARWGAtQsgWSl4vflVy2PFPEz0tv/bal8xa5meLMFrUKTX5hgUvYU/\n" \
"Z6tGn6D/Qqc6f1zLXbBwHSs09dR2CQzreExZBfMzQsNhFRAbd03OIozUhfJFfbdT\n" \
"6u9AWpQKXCBfTkBdYiJ23//OYb2MI3jSNwLgjt7RETeJ9r/tSQdirpLsQBqvFAnZ\n" \
"0E6yove+7u7Y/9waLd64NnHi/Hm3lCXRSHNboTXns5lndcEZOitHTtNCjv0xyBZm\n" \
"2tIMPNuzjsmhDYAPexZ3FL//2wmUspO8IFgV6dtxQ/PeEMMA3KgqlbbC1j+Qa3bb\n" \
"bP6MvPJwNQzcmRk13NfIRmPVNnGuV/u3gm3c\n" \
"-----END CERTIFICATE-----\n" \
;
//------------------------------------------------------------
void dualPrint(const __FlashStringHelper* text)
{
  
  lcd.print(text);
  Serial.println(text);
}

// piezo buzzer beep
void beep(int count=1)
{

ledcSetup(BUZZ_CHANNEL, 5000, 10);
ledcAttachPin(BUZZ_PIN, BUZZ_CHANNEL);
for (size_t j = 0; j < count; j++)
{
  /* code */
  if(j!=0)
  delay(300);
  for (int i = 200; i < 1000; i++)
  {
    ledcWrite(BUZZ_CHANNEL, i);
    delayMicroseconds(30);
  }
  ledcWrite(BUZZ_CHANNEL, 0);
}
  ledcDetachPin(BUZZ_PIN);
  pinMode(BUZZ_PIN,INPUT);
  
}

//sample action
void openDoor()
{
  digitalWrite(DOOR_PIN,HIGH);
  delay(2000);
  digitalWrite(DOOR_PIN,LOW);
}



void setup()
{

 pinMode(DOOR_PIN, OUTPUT);
 
  lcd.init();
  // Print a message to the LCD.
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Connecting wifi");
  lcd.setCursor(0, 1);
  Serial.begin(115200); // Initialize serial communications with the PC
  while (!Serial);             
  SPI.begin();        // Init SPI bus
  mfrc522.PCD_Init(); // Init MFRC522
  WiFi.begin(ssid, password);
  Serial.println(F("Connecting wifi"));
  byte printedDots=0;
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
      if(printedDots++>15)
    {
      for(;printedDots>=1;printedDots--)
      {
        lcd.setCursor(printedDots,1);
        lcd.print(' ');
      }
      lcd.setCursor(1,1);
    printedDots=1;
    }
    else
    lcd.print(".");
  }
  lcd.clear();
  lcd.setCursor(0,0);
  Serial.println("");
 dualPrint(F("WiFi connected"));
  lcd.setCursor(0,1);

  Serial.println(WiFi.localIP());
#ifdef OTA
  ArduinoOTA
      .onStart([]() {
        String type;
        if (ArduinoOTA.getCommand() == U_FLASH)
          type = "sketch";
        else // U_SPIFFS
          type = "filesystem";

        // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
        Serial.println("Start updating " + type);
      })
      .onEnd([]() {
        Serial.println("\nEnd");
      })
      .onProgress([](unsigned int progress, unsigned int total) {
        Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
      })
      .onError([](ota_error_t error) {
        Serial.printf("Error[%u]: ", error);
        if (error == OTA_AUTH_ERROR)
          Serial.println("Auth Failed");
        else if (error == OTA_BEGIN_ERROR)
          Serial.println("Begin Failed");
        else if (error == OTA_CONNECT_ERROR)
          Serial.println("Connect Failed");
        else if (error == OTA_RECEIVE_ERROR)
          Serial.println("Receive Failed");
        else if (error == OTA_END_ERROR)
          Serial.println("End Failed");
      });

  ArduinoOTA.begin();
#endif


  //lcd.clear();
  lcd.print("Ready");

  beep(2);
}




void clearDisplayIn(int mSec=5000)
{

   clearDisplayTimer=millis()+mSec;
   needDisplayUpdate=true;
 
}


void handleDataFromGoogle(String data)
{

  int colonIndex = data.indexOf(":");
  String accessType = data.substring(0, colonIndex);
  int nextColonIndex = data.indexOf(":", colonIndex + 1);
  String name = data.substring(colonIndex + 1, nextColonIndex);
  String text = data.substring(nextColonIndex + 1, data.length());
  
  lcd.setCursor(0,0);
  lcd.print("Hi ");
  lcd.print(name);
  lcd.setCursor(0,1);
  lcd.print(text);
    
  if(accessType.equalsIgnoreCase("beep"))
  {
    beep(5);
    
  }
  else if(accessType.equalsIgnoreCase("door"))
  {
    openDoor();
    
  }
}




void getGoogleData()
{
  HTTPClient http;
  String data;

  lcd.clear();
  uint64_t time = esp_timer_get_time();
  char url[150];
  int pointerShift=sprintf(url,"%s?uid=",mainLinkForSpr);

for (size_t i = 0; i < mfrc522.uid.size; i++)
{
    pointerShift+=sprintf(url+pointerShift,"%X",mfrc522.uid.uidByte[i]);

}

  
 
#ifdef TERMINAL_NAME
 pointerShift+=sprintf(url+pointerShift,"&terminal=%s",TERMINAL_NAME);
#endif

Serial.println(url);
Serial.println(F("Connecting to google"));
lcd.print(F("Connecting to"));
lcd.setCursor(7,1);
lcd.print(F("Google"));

//you need to make two request, the second request to a redirected url
//to get redirect url you need to read "Location" header
http.begin(url, root_ca);

  const char *location = "Location";
  const char *headerKeys[] = {location};
  http.collectHeaders(headerKeys, 1);
  int code = http.GET();
  Serial.printf("code %d\n", code);
  // 302 code means redirect
  if (code == 302)
  {

    String newUrl = http.header(location);

    http.end();

    Serial.println(newUrl);
    http.begin(newUrl, root_ca);
    code = http.GET();
    Serial.printf("status code %d\n", code);

 
    data=http.getString();
    Serial.println(data);

    lcd.clear();
    lcd.setCursor(0,0);

    lcd.setCursor(0,1);
    // lcd.print("ready to wrt for");
    // lcd.print(res);
   
    }
    else{
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print(code);
    if (code == 403|| code==-1)
    {
      lcd.clear();
    lcd.setCursor(0,0);
    lcd.print(F("Err open terminl"));
    lcd.setCursor(0,1);
    lcd.print("for help");
    if(code==-1){
Serial.println(F("If it says somethink like start_ssl_clien error"));
Serial.print(F("try to update the ssl CERTIFICATE"));
    }
    else{
    Serial.print(F("Open this link in any browser "));
    Serial.println(url);
    Serial.println(F("If it says Authorization is ...."));
    Serial.println(F("Open the google script and republish it"));
    }
    }
   
    
    else{
    lcd.print(F("Something wrong"));
    lcd.setCursor(0,1);
    }

    }
      
 
 
        
      if(!data.isEmpty() && data.length()>1)
      {
             handleDataFromGoogle(data);
      }

  Serial.printf("time=%d\n", esp_timer_get_time() - time);
  clearDisplayIn();
 
}



void loop()
{


if(needDisplayUpdate && millis()>clearDisplayTimer)
{
  
  lcd.clear();
  lcd.setCursor(1,1);
  lcd.print("Ready To Scan");
  needDisplayUpdate=false;
  lcd.noBacklight();
  //clearDisplayIn(1000);
   
}

#ifdef OTA
  ArduinoOTA.handle();
#endif
  if (!mfrc522.PICC_IsNewCardPresent())
  {
    return;
  }

  // Select one of the cards
  if (!mfrc522.PICC_ReadCardSerial())
  {
    return;
  }
beep();
lcd.backlight();

for (size_t i = 0; i < mfrc522.uid.size; i++)
{
  
  Serial.printf("%X",mfrc522.uid.uidByte[i]);
}
getGoogleData();
   
  mfrc522.PICC_HaltA();
  // Stop encryption on PCD
  mfrc522.PCD_StopCrypto1();
  beep();
}








