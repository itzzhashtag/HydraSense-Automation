/*
  Hello This is me Hashtag .....
  This is my Personal Home Automation Work, I use this project in my room for Refillling water in my Humidifier.
  Here I have connected the Relay with a Solenoid water Valve which turns on when the water level falls down.
  This Project has Proper Start and Stop MEchanism to avoid Spillage in case of any misshap ..!!
  Name: Aniket Chowdhury [Hashtag]
  Email: micro.aniket@gmail.com
  GitHub: https://github.com/itzzhashtag
  Instagram: https://instagram.com/itzz_hashtag
  LinkedIn: https://www.linkedin.com/in/itzz-hashtag/
*/
//Change Values and Data before using the code
//(Changes needed) Line Number -> 37, 38, 40

//===================================================================================================================
// --- Libraries Used  ---
//===================================================================================================================
#include <Wire.h>
#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
#include <avr/power.h>
#endif
#include <Adafruit_GFX.h>
#include <Adafruit_SH1106.h>
#define OLED_RESET 4
Adafruit_SH1106 display(OLED_RESET);
#define TRIG_PIN A3              // Ultrasonic Sensor Trigger Pin
#define ECHO_PIN A2              // Ultrasonic Sensor Echo Pin
#define NUM_LED 19              // no of leds
#define RELAY_PIN A1            // Relay data in

//===================================================================================================================
// ---  Variables and Constants ---
//===================================================================================================================
Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUM_LED, A0, NEO_GRB + NEO_KHZ800);// led pin, type -> //LED Strip Controller
Adafruit_NeoPixel strip2(1, 6, NEO_GRB + NEO_KHZ800);
const int minDistance = 4;  //          <--------------------------- Min distance (sensor to full water level) in cm 
const int maxDistance = 22; //          <--------------------------- Max distance (sensor to empty level) in cm  
int currentLitLeds = 0;  // Global: Keeps track of current visible LEDs
int max_brig=200; //                    <--------------------------- Max Brightness to set UP here..!!!
int tm=0;
bool isFilling = false;

//===================================================================================================================
// --- Setup ---
//===================================================================================================================
void setup()
{
  strip.begin();
  strip.show();
  strip2.begin();
  strip2.setPixelColor(0, strip2.Color(255, 0, 0)); // Set to red
  strip2.show();
  
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(RELAY_PIN, OUTPUT); // Set the relay pin as output
  digitalWrite(RELAY_PIN, HIGH);
  Serial.begin(9600);
  display.begin(SH1106_SWITCHCAPVCC, 0x3C);
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.clearDisplay();
  display.setTextSize(2);
  display.setCursor(10, 20);
  display.print(" HASHTAG");
  display.display();
  display.setCursor(0, 45);
  display.setTextSize(1);
  display.print("       Welcome");
  display.display();
  delay(1500);
  rainbow(20);
}

//===================================================================================================================
// --- Loop ---
//===================================================================================================================
void loop()
{
  int distance = getDistance();
  if (distance < minDistance || distance > maxDistance + 10)
  {
    Serial.println("Out of range");
    Serial.println(distance);
    display.clearDisplay();
    display.setCursor(0, 20);
    display.setTextSize(2);
    display.print(" Error 500");
    display.display();
    display.setCursor(0, 45);
    display.setTextSize(1);
    display.print("   H2O Sensor Error");
    errorLED();
    digitalWrite(RELAY_PIN, HIGH);
    display.display();
    delay(1000);
    return;
  }
  int waterLevel = chk_lvl();
  if (waterLevel < 6 || waterLevel > 90)
  {
    controlRelay(waterLevel); // Handle relay based on level
  }
  if (isFilling)
  {
    updateLED(waterLevel);
    delay(100);
    return;
  }
  updateLED(waterLevel);
  displayWaterLevel(waterLevel);
  delay(1000);
}

//===================================================================================================================
// --- Measures the Distance between Water Level and Sonar Sensor ---
//===================================================================================================================
int chk_lvl()
{
  int distance = getDistance();
  int waterLevel = map(distance, minDistance, maxDistance, 100, 0);
  waterLevel = constrain(waterLevel, 0, 100);
  Serial.print("Water Level: ");
  Serial.print(waterLevel);
  Serial.println("%");
  Serial.print(distance);
  Serial.println(" cm");
  if (distance < minDistance || distance > maxDistance + 20)
  {
    Serial.println("Out of range");
    Serial.println(distance);
    display.clearDisplay();
    display.setCursor(0, 20);
    display.setTextSize(2);
    display.print(" Error 402");
    display.display();
    display.setCursor(0, 45);
    display.setTextSize(1);
    display.print("   Refilling Error");
    errorLED();
    display.display();
    delay(1000);
    digitalWrite(RELAY_PIN, HIGH);
    chk_lvl();
  }
  return waterLevel;
}

//===================================================================================================================
// --- Function to Refill Water and keep on this Function untill Water is not Refilled  ---
//===================================================================================================================
void controlRelay(int level)
{
  if (level < 10)
  {
    isFilling = true;
    while (isFilling)
    {
      int level2 = chk_lvl();
      digitalWrite(RELAY_PIN, LOW);
      display.clearDisplay();
      display.setCursor(0, 10);
      display.setTextSize(2);
      display.print(" Refilling");
      Serial.println("Refilling ");
      displayRefillingScreen(level2);
      display.display();
      display.setCursor(0, 35);
      display.setTextSize(1);
      display.print("     Please Wait");
      errorLED();
      display.display();
      delay(1000);
      /*if(tm>30)
      {
        digitalWrite(RELAY_PIN, HIGH);
        tm=0;
        delay(500);
      }
      tm++;
      Serial.println(tm);
      */
      if (level2 >= 90)
      {
        isFilling = false;
      }
    }
  }
  else if (level >= 90)
  {
    digitalWrite(RELAY_PIN, HIGH);   // Stop filling
    isFilling = false;
  }
}

//===================================================================================================================
// --- Function to measure distance using HC-SR04 Sensor ---
//===================================================================================================================
int getDistance() 
{
  long total = 0;
  for (int i = 0; i < 5; i++) //does average of the Distance Calculated
  {
    digitalWrite(TRIG_PIN, LOW);
    delayMicroseconds(2);
    digitalWrite(TRIG_PIN, HIGH);
    delayMicroseconds(10);
    digitalWrite(TRIG_PIN, LOW);
    long duration = pulseIn(ECHO_PIN, HIGH);
    total += duration;
    delay(10); // Short pause between samples
  }
  long avgDuration = total / 5;
  return avgDuration * 0.034 / 2;
}


//===================================================================================================================
// --- Display OLED Code ---
//===================================================================================================================
void displayWaterLevel(int level)     // Function to display water level on OLED
{
  display.clearDisplay();
  display.setCursor(20, 5);
  display.setTextSize(1);
  display.print(" H2O");
  display.setCursor(20, 15);
  display.print("Level");
  display.setCursor(0, 30);
  display.setTextSize(3);
  display.print(level);
  display.print("%");
  // Tank Position
  int tankX = 72;  // Shifted to center
  int tankY = 5;
  int tankWidth = 55;  // Increased width
  int tankHeight = 50;
  // Draw tank body with rounded bottom
  display.drawRoundRect(tankX, tankY, tankWidth, tankHeight, 5, WHITE);
  // Draw tank lid
  display.drawLine(tankX + 5, tankY - 3, tankX + tankWidth - 5, tankY - 3, WHITE);
  display.drawLine(tankX + 5, tankY - 3, tankX, tankY, WHITE);
  display.drawLine(tankX + tankWidth - 5, tankY - 3, tankX + tankWidth, tankY, WHITE);
  // Draw inlet pipe on top left
  display.drawLine(tankX + 8, tankY - 6, tankX + 8, tankY - 12, WHITE);
  display.drawLine(tankX + 8, tankY - 12, tankX + 14, tankY - 12, WHITE);
  display.drawLine(tankX + 14, tankY - 12, tankX + 14, tankY - 6, WHITE);
  // Draw water level inside the tank
  int waterHeight = map(level, 0, 100, 0, tankHeight);
  display.fillRoundRect(tankX + 2, tankY + tankHeight - waterHeight, tankWidth - 4, waterHeight, 5, WHITE);
  display.display();
}

//===================================================================================================================
// --- LED Blinking Code (Red-WARNING) ---
//===================================================================================================================
void errorLED()
{
  // Fade in
  for (int b = 0; b <= 255; b += 5)
  {
    for (int i = 0; i < NUM_LED; i++)
    {
      strip.setPixelColor(i, strip.Color(b, 0, 0));
    }
    strip.show();
    delay(10);
  }
  // Fade out
  for (int b = 255; b >= 0; b -= 5) {
    for (int i = 0; i < NUM_LED; i++) {
      strip.setPixelColor(i, strip.Color(b, 0, 0));
    }
    strip.show();
    delay(10);
  }
}

//===================================================================================================================
// --- LED Indicator of water Level ---
//===================================================================================================================
void updateLED(int level)
{
  int targetLeds = map(level, 0, 100, 0, NUM_LED);  // Number of LEDs based on level
  uint8_t r = map(level, 0, 100, 255, 0);
  uint8_t g = map(level, 0, 100, 0, 200);
  uint8_t b = map(level, 0, 100, 0, 255);

  if (level < 25)
  {
    // Below 20%: blinking/fading LEDs
    for (int fade = 0; fade <= 255; fade += 10)
    {
      for (int i = 0; i < targetLeds; i++)
      {
        strip.setPixelColor(i, strip.Color((r * fade) / 255, (g * fade) / 255, (b * fade) /255));
      }
      strip.show();
      delay(10);
    }
    for (int fade = 255; fade >= 0; fade -= 10)
    {
      for (int i = 0; i < targetLeds; i++)
      {
        strip.setPixelColor(i, strip.Color((r * fade) / 255, (g * fade) / 255, (b * fade) / 255));
      }
      strip.show();
      delay(10);
    }

    // Turn off the rest of the LEDs
    for (int i = targetLeds; i < NUM_LED; i++)
    {
      strip.setPixelColor(i, 0);
    }
    strip.show();
    currentLitLeds = targetLeds;  // Update to match
  }
  else
  {
    // 20% or more: Smooth transitions without blinking
    if (currentLitLeds < targetLeds)
    {
      for (int i = currentLitLeds; i < targetLeds; i++)
      {
        for (int brightness = 0; brightness <= max_brig; brightness += 15)
        {
          strip.setPixelColor(i, strip.Color((r * brightness) / 255, (g * brightness) / max_brig, (b * brightness) / 255));
          strip.show();
          delay(5);
        }
      }
    }
    else if (currentLitLeds > targetLeds)
    {
      for (int i = currentLitLeds - 1; i >= targetLeds; i--)
      {
        for (int brightness = max_brig; brightness >= 0; brightness -= 15)
        {
          strip.setPixelColor(i, strip.Color((r * brightness) / max_brig, (g * brightness) / max_brig, (b * brightness) / max_brig));
          strip.show();
          delay(5);
        }
        strip.setPixelColor(i, 0);  // Turn off completely
      }
    }
    // Ensure all LEDs up to target are solid
    for (int i = 0; i < targetLeds; i++)
    {
      strip.setPixelColor(i, strip.Color(r, g, b));
    }
    // Turn off the rest
    for (int i = targetLeds; i < NUM_LED; i++)
    {
      strip.setPixelColor(i, 0);
    }
    strip.show();
    currentLitLeds = targetLeds;
  }
}

//===================================================================================================================
// --- Fancy Start Rainbow for LED ---
//===================================================================================================================
void rainbow(uint8_t wait)
{
  uint16_t i, j;

  for (j = 0; j < 256; j++)
  {
    for (i = 0; i < strip.numPixels(); i++)
    {
      strip.setPixelColor(i, Wheel((i + j) & 255));
    }
    strip.show();
    delay(wait);
  }
}
uint32_t Wheel(byte WheelPos)
{
  WheelPos = 255 - WheelPos;
  if (WheelPos < 85)
  {
    return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  }
  if (WheelPos < 170)
  {
    WheelPos -= 85;
    return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
  WheelPos -= 170;
  return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
}

//===================================================================================================================
// --- Display OLED the Water Refilling ---
//===================================================================================================================
void displayRefillingScreen(int level)
{
    // Progress Bar
    int barWidth = map(level, 0, 100, 0, 120); // Map level (0-100%) to bar width (0-120px)
    int barX = 5;
    int barY = 55;
    int barHeight = 8;
    display.drawRect(barX, barY, 120, barHeight, WHITE); // Outline
    display.fillRect(barX, barY, barWidth, barHeight, WHITE); // Filled part
    display.display();
}
//===================================================================================================================
// --- THE END ---
//===================================================================================================================
