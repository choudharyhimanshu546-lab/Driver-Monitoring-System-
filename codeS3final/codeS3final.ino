#include <WiFi.h>
#include <HTTPClient.h>
#include <Adafruit_NeoPixel.h>

/* WIFI */
const char* ssid = "*******";
const char* password = "*********";

/* API */
const char* server = "ADD API";

/* DEVICE AUTH */
String deviceId = "***********";
String secret   = "************";

/* MQ3 SENSOR */
#define MQ3_PIN 4
#define ALCOHOL_THRESHOLD 400

/* RGB LED */
#define RGB_PIN 48
Adafruit_NeoPixel pixel(1, RGB_PIN, NEO_GRB + NEO_KHZ800);

/* LOCATION */
float latitude  = 30.709331;
float longitude = 76.689283;

/* TIMER */
unsigned long lastSend = 0;
const long sendInterval = 120000;   // 2 minutes

/* WIFI CONNECT */
void connectWiFi()
{
  Serial.print("🌐 Connecting WiFi");

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\n✅ WiFi Connected");
  Serial.print("📡 IP: ");
  Serial.println(WiFi.localIP());
}

/* RGB CONTROL */
void setColor(int r,int g,int b)
{
  pixel.setPixelColor(0, pixel.Color(r,g,b));
  pixel.show();
}

/* READ MQ3 SENSOR */
int readAlcohol()
{
  int total = 0;

  for(int i=0;i<10;i++)
  {
    total += analogRead(MQ3_PIN);
    delay(5);
  }

  return total / 10;
}

/* SEND DATA TO API */
void sendData(int alcoholLevel,bool alert)
{
  if(WiFi.status()!=WL_CONNECTED)
  {
    Serial.println("⚠ WiFi lost. Reconnecting...");
    connectWiFi();
  }

  HTTPClient http;

  http.begin(server);

  http.addHeader("Content-Type","application/json");
  http.addHeader("deviceid",deviceId);
  http.addHeader("secret",secret);

  String payload="{";
  payload += "\"latitude\":" + String(latitude,6) + ",";
  payload += "\"longitude\":" + String(longitude,6) + ",";
  payload += "\"alcohol\":" + String(alcoholLevel) + ",";
  payload += "\"alert\":" + String(alert ? "true":"false");
  payload += "}";

  Serial.println("\n🚀 Sending Data To API");
  Serial.println("📤 Payload:");
  Serial.println(payload);

  int httpCode=http.POST(payload);

  Serial.print("📩 Response Code: ");
  Serial.println(httpCode);

  if(httpCode==200)
  {
    Serial.println("✅ DATA SENT SUCCESSFULLY");

    String response=http.getString();

    Serial.println("📨 Server Response:");
    Serial.println(response);
  }
  else
  {
    Serial.println("❌ API SEND FAILED");
  }

  http.end();
}

/* SETUP */
void setup()
{
  Serial.begin(115200);

  pinMode(MQ3_PIN,INPUT);

  pixel.begin();
  pixel.show();

  Serial.println("🔥 MQ3 Warming...");
  delay(20000);

  connectWiFi();

  /* SEND FIRST DATA IMMEDIATELY */
  lastSend = millis() - sendInterval;
}

/* LOOP */
void loop()
{
  int alcoholLevel = readAlcohol();

  bool alert = alcoholLevel > ALCOHOL_THRESHOLD;

  Serial.println("\n==========================");

  Serial.print("📊 Alcohol Value: ")/.ldfop.,mnn,,aqw;
  Serial.println(alcoholLevel);

  Serial.print("⚠ Alcohol Alert: ");
  Serial.println(alert ? "YES":"NO");

  Serial.print("📍 Latitude: ");
  Serial.println(latitude,6);

  Serial.print("📍 Longitude: ");
  Serial.println(longitude,6);

  Serial.println("==========================");

  /* RGB STATUS */

  if(alert)
    setColor(255,0,0);   // RED
  else
    setColor(0,255,0);   // GREEN

  /* SEND DATA EVERY 2 MINUTES */

  if(millis() - lastSend >= sendInterval)
  {
    sendData(alcoholLevel,alert);

    lastSend = millis();
  }

  delay(1000);
}
