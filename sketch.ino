#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

Adafruit_MPU6050 mpu;

// 74HC595 Control Pins (Moved Clock to GPIO 4 for reliable simulation stability)
const int DATA_PIN  = 15;  // DS (Pin 14 on 74HC595)
const int CLOCK_PIN = 4;   // SH_CP (Pin 11 on 74HC595)
const int LATCH_PIN = 16;  // ST_CP (Pin 12 on 74HC595)

const int BUZZER_PIN = 17;
const int NUM_ZONES = 6;

const int trigPins[NUM_ZONES] = {13, 26, 14, 33, 19, 23};
const int echoPins[NUM_ZONES] = {12, 25, 27, 32, 34, 2};

const char* zoneNames[NUM_ZONES] = {
  "FRONT SENSOR",
  "FRONT LEFT SENSOR",
  "FRONT RIGHT SENSOR",
  "REAR SENSOR",
  "REAR LEFT SENSOR",
  "REAR RIGHT SENSOR"
};

void setup() {
  Serial.begin(115200);

  pinMode(DATA_PIN, OUTPUT);
  pinMode(CLOCK_PIN, OUTPUT);
  pinMode(LATCH_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);

  for (int i = 0; i < NUM_ZONES; i++) {
    pinMode(trigPins[i], OUTPUT);
    pinMode(echoPins[i], INPUT);
  }

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    for (;;);
  }
  display.clearDisplay();
  display.display();

  if (!mpu.begin()) {
    Serial.println("MPU6050 not found");
  }
  mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
  mpu.setGyroRange(MPU6050_RANGE_500_DEG);
  mpu.setFilterBandwidth(MPU6050_BAND_260_HZ);
  delay(100);
}

float readDistance(int trigPin, int echoPin) {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  
  long duration = pulseIn(echoPin, HIGH, 30000);
  if (duration == 0) return 999.0;
  return duration * 0.0343 / 2.0;
}

void updateIndicators(uint8_t reg1, uint8_t reg2) {
  digitalWrite(LATCH_PIN, LOW);
  shiftOut(DATA_PIN, CLOCK_PIN, MSBFIRST, reg2);
  shiftOut(DATA_PIN, CLOCK_PIN, MSBFIRST, reg1);
  digitalWrite(LATCH_PIN, HIGH);
}

void loop() {
  float distances[NUM_ZONES];
  int closestZoneIndex = -1;
  float minDistance = 999.0;

  for (int i = 0; i < NUM_ZONES; i++) {
    distances[i] = readDistance(trigPins[i], echoPins[i]);
    if (distances[i] < minDistance) {
      minDistance = distances[i];
      closestZoneIndex = i;
    }
    delay(5);
  }

  uint8_t r1_state = 0;
  uint8_t r2_state = 0;

  for (int i = 0; i < NUM_ZONES; i++) {
    bool greenOn = false;
    bool redOn = false;

    if (distances[i] > 50.0) {
      greenOn = true;
    } else if (distances[i] >= 30.0 && distances[i] <= 50.0) {
      greenOn = true;
      redOn = true; // Green + Red combines physically to display Yellow
    } else {
      redOn = true;
    }

    switch(i) {
      case 0:
        if (greenOn) r1_state |= (1 << 2);
        if (redOn)   r1_state |= (1 << 1);
        break;
      case 1:
        if (greenOn) r1_state |= (1 << 4);
        if (redOn)   r1_state |= (1 << 3);
        break;
      case 2:
        if (greenOn) r1_state |= (1 << 6);
        if (redOn)   r1_state |= (1 << 5);
        break;
      case 3:
        if (greenOn) r2_state |= (1 << 0);
        if (redOn)   r1_state |= (1 << 7);
        break;
      case 4:
        if (greenOn) r2_state |= (1 << 2);
        if (redOn)   r2_state |= (1 << 1);
        break;
      case 5:
        if (greenOn) r2_state |= (1 << 4);
        if (redOn)   r2_state |= (1 << 3);
        break;
    }
  }

  updateIndicators(r1_state, r2_state);

  if (closestZoneIndex != -1 && minDistance < 30.0) {
    tone(BUZZER_PIN, 1500);

    display.clearDisplay();
    display.setTextColor(SSD1306_WHITE);
    display.drawTriangle(64, 2, 49, 27, 79, 27, SSD1306_WHITE);
    display.setTextSize(2);
    display.setCursor(61, 10);
    display.print("!");
    
    display.setTextSize(1);
    display.setCursor(0, 36);
    display.print("ALARM: ");
    display.print(zoneNames[closestZoneIndex]);
    display.setCursor(0, 50);
    display.print("DISTANCE: ");
    display.print(minDistance, 1);
    display.print(" cm");
    display.display();
  } 
  else {
    noTone(BUZZER_PIN);

    sensors_event_t a, g, temp;
    mpu.getEvent(&a, &g, &temp);

    display.clearDisplay();
    display.setTextColor(SSD1306_WHITE);
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.print("SYSTEM: MONITORING");
    display.drawFastHLine(0, 10, 128, SSD1306_WHITE);
    
    display.setCursor(0, 18);
    display.print("Closest: ");
    if (minDistance > 150.0) display.print("CLEAR");
    else {
      display.print(minDistance, 1);
      display.print("cm");
    }

    display.setCursor(0, 35);
    display.print("Tilt X: ");
    display.print(a.acceleration.x, 2);
    display.print(" m/s2");
    
    display.setCursor(0, 48);
    display.print("Tilt Y: ");
    display.print(a.acceleration.y, 2);
    display.print(" m/s2");
    display.display();
  }

  delay(40);
}