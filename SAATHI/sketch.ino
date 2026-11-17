#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <OneWire.h>
#include <DallasTemperature.h>

// SAATHI educational simulator
// DS18B20 is used ONLY as a body-temperature simulation substitute.
// The planned real device uses the MLX90614 non-contact IR sensor.
// Pulse and SpO2 values below are simulated with potentiometers.
// This is not a medical device and must not be used for health decisions.

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1

const int ONE_WIRE_BUS = 15;
const int PULSE_KNOB_PIN = 34;
const int SPO2_KNOB_PIN = 35;

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature bodyTempSensor(&oneWire);

void setup() {
  Serial.begin(115200);
  Wire.begin(21, 22);
  analogReadResolution(12);
  bodyTempSensor.begin();

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("OLED not found. Check I2C wiring.");
    while (true) delay(10);
  }

  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println("SAATHI BODY TEMP");
  display.println("Starting demo...");
  display.display();
  delay(1200);
}

void loop() {
  bodyTempSensor.requestTemperatures();
  float simulatedBodyTemp = bodyTempSensor.getTempCByIndex(0);

  int pulseRaw = analogRead(PULSE_KNOB_PIN);
  int spo2Raw = analogRead(SPO2_KNOB_PIN);

  // These are example values controlled by potentiometers, not measurements.
  int simulatedPulse = map(pulseRaw, 0, 4095, 40, 180);
  int simulatedSpO2 = map(spo2Raw, 0, 4095, 85, 100);

  bool tempValid = (simulatedBodyTemp != DEVICE_DISCONNECTED_C &&
                    simulatedBodyTemp > -50 && simulatedBodyTemp < 100);

  display.clearDisplay();
  display.setCursor(0, 0);
  display.setTextSize(1);
  display.println("SAATHI | SIM ONLY");
  display.println("---------------------");

  display.print("Body temp* : ");
  if (tempValid) {
    display.print(simulatedBodyTemp, 1);
    display.println(" C");
  } else {
    display.println("Sensor error");
  }

  display.print("Pulse SIM  : ");
  display.print(simulatedPulse);
  display.println(" bpm");

  display.print("SpO2 SIM   : ");
  display.print(simulatedSpO2);
  display.println(" %");

  display.println("---------------------");
  display.println("*DS18B20 test sensor");
  display.display();

  Serial.println("===== SAATHI DEMO =====");
  if (tempValid) {
    Serial.printf("Simulated body-temp input: %.1f C\n", simulatedBodyTemp);
  } else {
    Serial.println("Temperature sensor read error");
  }
  Serial.printf("Simulated pulse: %d bpm\n", simulatedPulse);
  Serial.printf("Simulated SpO2: %d %%\n\n", simulatedSpO2);

  delay(1000);
}
