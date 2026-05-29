/*
raindrop module analog output for graph

Wire Connection
1. Rain Sensor Module	Arduino	Notes
VCC	5V	Power supply
GND	GND	Ground
DO (digital out)	D2 (example)	HIGH when dry, LOW when wet
AO (analog out)	A0 (optional)	Returns analog moisture level
2. OLED wire connecction
Wiring: I2C OLED
OLED Pin	Arduino Uno / Mega
VCC	5V
GND	GND
SDA	A4 (Uno) / 20 (Mega)
SCL	A5 (Uno) / 21 (Mega)
*/

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define RAIN_SENSOR_AO_PIN A0
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// Histogram settings
#define MAX_BARS 50
#define BAR_WIDTH 2
#define SAMPLE_INTERVAL 100     // ms
#define SAMPLE_WINDOW 2000      // 2 seconds
#define SAMPLES_PER_BAR (SAMPLE_WINDOW / SAMPLE_INTERVAL)

uint8_t barHeights[MAX_BARS] = {0}; // Histogram values
int currentBarIndex = 0;

int rainSamples[SAMPLES_PER_BAR];
int sampleCount = 0;
unsigned long lastSampleTime = 0;

const int MAX_ANALOG = 700;

void setup() {
  Serial.begin(9600);
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("OLED not found"));
    while (true);
  }

  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println("Rain Histogram Init");
  display.display();
  delay(1000);
}

void loop() {
  if (millis() - lastSampleTime >= SAMPLE_INTERVAL) {
    lastSampleTime = millis();

    // Read and clamp rain value
    int rainValue = analogRead(RAIN_SENSOR_AO_PIN);
    rainValue = constrain(rainValue, 0, MAX_ANALOG);
    rainSamples[sampleCount++] = rainValue;

    // Once we have enough samples for a bar
    if (sampleCount >= SAMPLES_PER_BAR) {
      // Compute average
      long sum = 0;
      for (int i = 0; i < SAMPLES_PER_BAR; i++) {
        sum += rainSamples[i];
      }
      int avg = sum / SAMPLES_PER_BAR;

      // Map to display height
      uint8_t barHeight = map(avg, 0, MAX_ANALOG, 0, SCREEN_HEIGHT - 1);

      // Shift bars left
      for (int i = 0; i < MAX_BARS - 1; i++) {
        barHeights[i] = barHeights[i + 1];
      }
      barHeights[MAX_BARS - 1] = barHeight;

      sampleCount = 0; // Reset for next bar
    }

    // Draw histogram
    display.clearDisplay();

    // Y-axis labels
    display.setCursor(0, 0); display.print(MAX_ANALOG);
    display.setCursor(0, SCREEN_HEIGHT / 2 - 3); display.print(MAX_ANALOG / 2);
    display.setCursor(0, SCREEN_HEIGHT - 8); display.print("0");

    // Axis lines
    display.drawLine(20, 0, 20, SCREEN_HEIGHT, SSD1306_WHITE);
    display.drawLine(20, SCREEN_HEIGHT - 1, SCREEN_WIDTH, SCREEN_HEIGHT - 1, SSD1306_WHITE);

    // Draw histogram bars
    for (int i = 0; i < MAX_BARS; i++) {
      int x = 22 + i * BAR_WIDTH;
      int y = SCREEN_HEIGHT - barHeights[i];
      display.fillRect(x, y, BAR_WIDTH - 1, barHeights[i], SSD1306_WHITE);
    }

    // Show latest rain value
    display.setCursor(70, 0);
    display.print("Rain:");
    display.print(rainSamples[sampleCount - 1]);

    display.display();
  }
}
