#include <Adafruit_NeoPixel.h>
#include <Adafruit_SSD1306.h>
#include <DHT.h>
#include <Wire.h>

// ------------------------
// CONFIGURATION
// ------------------------
#define DHTPIN 26
#define DHTTYPE DHT22

#define MQ2PIN 34
#define PIRPIN 19
#define BUZZER 33
#define NEOPIXEL_PIN 2
#define NUMPIXELS 12

// OLEDs
#define OLED1_SDA 21  // Fire info
#define OLED1_SCL 37
#define OLED2_SDA 39  // Sensor logs
#define OLED2_SCL 38
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

#define TEMP_FIRE 45
#define SMOKE_FIRE 300

// ------------------------
// OBJECTS
// ------------------------
DHT dht(DHTPIN, DHTTYPE);
Adafruit_NeoPixel strip(NUMPIXELS, NEOPIXEL_PIN, NEO_GRB + NEO_KHZ800);

// ESP32 has built-in Wire1 for second I2C
Adafruit_SSD1306 oled1(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);   // Fire info
Adafruit_SSD1306 oled2(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire1, -1);  // Sensor logs

// ------------------------
// TIMING & STATE
// ------------------------
unsigned long previousSensorMillis = 0;
const long sensorInterval = 200; // Sensor read interval

// Sensor values
float temp = 0;
float hum = 0;
int smoke = 0;
bool motion = false;
bool fireDetected = false;
bool lastFireState = false;

// ------------------------
// SETUP
// ------------------------
void setup() {
  Serial.begin(115200);

  // Initialize sensors
  dht.begin();
  pinMode(MQ2PIN, INPUT);
  pinMode(PIRPIN, INPUT);
  pinMode(BUZZER, OUTPUT);

  // NeoPixel
  strip.begin();
  strip.show(); // turn off all pixels initially

  // Initialize OLED1 (Fire info)
  Wire.begin(OLED1_SDA, OLED1_SCL);
  if(!oled1.begin(SSD1306_SWITCHCAPVCC, 0x3C)){
    Serial.println("OLED1 failed");
    while(true);
  }
  oled1.clearDisplay();
  oled1.display();

  // Initialize OLED2 (Sensor logs)
  Wire1.begin(OLED2_SDA, OLED2_SCL);
  if(!oled2.begin(SSD1306_SWITCHCAPVCC, 0x3C)){
    Serial.println("OLED2 failed");
    while(true);
  }
  oled2.clearDisplay();
  oled2.display();
}

// ------------------------
// MAIN LOOP
// ------------------------
void loop() {
  unsigned long currentMillis = millis();

  // --- Task 1: Read sensors & update displays ---
  if(currentMillis - previousSensorMillis >= sensorInterval){
    previousSensorMillis = currentMillis;

    // Read sensors
    temp = dht.readTemperature();
    hum = dht.readHumidity();
    smoke = analogRead(MQ2PIN);
    motion = digitalRead(PIRPIN);

    fireDetected = (smoke >= SMOKE_FIRE) || (temp >= TEMP_FIRE);

    // Buzzer
    digitalWrite(BUZZER, (fireDetected && !motion) ? HIGH : LOW);

    // OLED1: Fire info + project credit
    oled1.clearDisplay();
    oled1.setTextSize(1);
    oled1.setTextColor(SSD1306_WHITE, SSD1306_BLACK);
    oled1.setCursor(0,0);
    oled1.println("FIRE SAFETY");
    oled1.println("-----------------");
    oled1.print("Fire: "); oled1.println(fireDetected ? "ALERT!" : "Normal");
    oled1.print("Motion: "); oled1.println(motion ? "Detected" : "None");
    oled1.print("Buzzer: "); oled1.println((fireDetected && !motion) ? "ON" : "OFF");
    oled1.setCursor(0, SCREEN_HEIGHT-10);
    oled1.println("By Hariom Sharnam");
    oled1.display();

    // OLED2: Sensor logs
    oled2.clearDisplay();
    oled2.setTextSize(1);
    oled2.setTextColor(SSD1306_WHITE, SSD1306_BLACK);
    oled2.setCursor(0,0);
    oled2.println("SENSOR LOG:");
    oled2.print("Temp: "); oled2.print(temp); oled2.println(" C");
    oled2.print("Humidity: "); oled2.print(hum); oled2.println(" %");
    oled2.print("Smoke: "); oled2.print(smoke); oled2.println(" ");
    oled2.print("Motion: "); oled2.println(motion ? "YES" : "NO");
    oled2.display();
  }

  // --- Task 2: NeoPixel full ring ---
  if(fireDetected){
    // Fire detected → full ring glows red
    for(int i = 0; i < NUMPIXELS; i++) {
      strip.setPixelColor(i, strip.Color(255,0,0)); // red
    }
    strip.show();
    lastFireState = true;
  } else {
    // Normal → full ring glows green
    if(lastFireState){ // update only once when state changes
      for(int i = 0; i < NUMPIXELS; i++) {
        strip.setPixelColor(i, strip.Color(0,255,0)); // green
      }
      strip.show();
      lastFireState = false;
    }
  }
}
