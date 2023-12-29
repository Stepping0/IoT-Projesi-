#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_MPU6050.h>
#include <ESP8266WiFi.h>
#include "ThingSpeak.h"
//ağ bilgileri
const char* ssid="*******************";
const char* password="**********";
const char* host = "maker.ifttt.com";
String url="/trigger/sarsinti_tespit/json/with/key/******************";
String butonURL="/trigger/içerde biri var/json/with/key/*************";

const char* apiKey="**********************";
const char* server="api.thingspeak.com";
unsigned long channelID = *********;             // Thingspeak channel ID 
unsigned int field_no=1;


//iftt
#define SDA_PIN D2  // SDA pini
#define SCL_PIN D1  // SCL pini
#define LED D0
#define BUTTON D3
Adafruit_MPU6050 mpu;

WiFiClient client;
WiFiClient client1;

void wifiSetup ()
{ 
  delay (10); 
  Serial.println(); Serial.println(ssid); 
  Serial.println(F("Kablosuz Agina Baglaniyor")); 
  WiFi.begin(ssid, password); 
  // WiFi durum kontrolü 
  while (WiFi.status() != WL_CONNECTED)
  { 
    delay(500); 
    Serial.print("."); 
  } 
  Serial.println(); 
  Serial.print(ssid); Serial.println("Kablosuz Aga Baglandi"); 
  Serial.println("IP adresi: "); Serial.println(WiFi.localIP()); 
}

void setup() 
{
  Serial.begin(115200);
  Wire.begin(SDA_PIN, SCL_PIN);
  
  if (!mpu.begin()) 
  {
    Serial.println("MPU6050 bulunamadı. Lütfen bağlantıları kontrol edin.");
    while (1);
  }
  mpu.setGyroRange(MPU6050_RANGE_500_DEG);
  Serial.println("MPU6050 başlatıldı!");

  pinMode(LED, OUTPUT);
  pinMode(BUTTON,INPUT_PULLUP);
  digitalWrite(LED, LOW);


//////////////////////////////////////////////////

  wifiSetup();          // Kablosuz ağ kurulum fonksiyonu
  ThingSpeak.begin(client1);         // ThingSpeak client sınıfı başlatılıyor
 
}



void loop() 
{
  const int httpPort=80;
  if(!client.connect(host,httpPort))
  {
    Serial.println("connection failed");
    return;
  }
  static bool lastButtonState = HIGH;

  // Buton durumunu oku
  bool buttonState = digitalRead(BUTTON);

  // Buton durumu değiştiyse
  if (buttonState != lastButtonState) {
    if (buttonState == LOW ) {  // Buton basıldıysa
      // Buton basıldığında ekrana mesaj yazdır
      Serial.println("Buton basıldı!");
      Serial.println(url);
      client.print(String("GET ") + butonURL + " HTTP/1.1\r\n" +
                   "Host: " + host + "\r\n" +
                   "Connection: close\r\n\r\n");
                   
    }
  }

  // Sonraki döngü için buton durumunu sakla
  lastButtonState = buttonState;

   sensors_event_t g;
   mpu.getGyroSensor()->getEvent(&g);

  float gyroX = g.gyro.x;
  float gyroY = g.gyro.y;
  float gyroZ = g.gyro.z;

   static bool firstReading = true;
  static float prevGyroX = 0.0;
  static float prevGyroY = 0.0;
  static float prevGyroZ = 0.0;

  Serial.print("Eksen verisi (X,Y,Z): ");
  Serial.print(gyroX);
  Serial.print(", ");
  Serial.print(gyroY);
  Serial.print(", ");
  Serial.println(gyroZ);

 
  // Önceki veriyi karşılaştır
  if (!firstReading && (abs(gyroX - prevGyroX) > 0.03 || abs(gyroY - prevGyroY) > 0.03 || abs(gyroZ - prevGyroZ) > 0.03)) {
    Serial.println("Sarsıntı tespit edildi!");
    digitalWrite(LED, HIGH);
    ///tone(BUZZER, 500); // 1000 Hz frekansında ses çal
    delay(5000); // 5 saniye boyunca sesi çal
    // LED'i kapat
    digitalWrite(LED, LOW);
  int an=1;
  ThingSpeak.writeField (channelID, field_no, an, apiKey);    
    ThingSpeak.setField (1, an);       // 1 nolu field ı kur 
 ThingSpeak.writeFields(channelID, apiKey);       // kurulu field lere yaz (çoklu yazma) 
  Serial.println("\n");


    Serial.println(url);
    client.print(String("GET ") + url + " HTTP/1.1\r\n" +
                   "Host: " + host + "\r\n" +
                   "Connection: close\r\n\r\n");
                   
  }
    ThingSpeak.writeFields(channelID, apiKey);
      ThingSpeak.setField (1, 0);       

  
  firstReading = false;

   // Önceki veriyi güncelle
  prevGyroX = gyroX;
  prevGyroY = gyroY;
  prevGyroZ = gyroZ;

  delay(1000); 
}