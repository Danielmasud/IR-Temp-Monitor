/**
 * @file IR_Temp_Monitor.ino
 * @author Daniel Masud
 * @brief Remote-Controlled Temperature Monitor using LM35, IR Receiver, and 16x2 LCD.
 * @date 2026-04-01
 */

#include <IRremote.h>
#include <LiquidCrystal.h>

// --- Hardware Pin Definitions ---
// LCD Data Pins: RS, E, D4, D5, D6, D7
const int LCD_RS = 7;
const int LCD_E  = 12;
const int LCD_D4 = 11;
const int LCD_D5 = 10;
const int LCD_D6 = 9;
const int LCD_D7 = 8;

const int IR_RECV_PIN = 2; 
const int LM35_PIN = A0;   // Analog input for temperature sensor

// --- IR Remote Codes ---
// The green button (Play/Pause) on the Car MP3 remote
const unsigned long CMD_POWER_TOGGLE = 0xBC43FF00; 

// --- Global Objects & State Variables ---
LiquidCrystal lcd(LCD_RS, LCD_E, LCD_D4, LCD_D5, LCD_D6, LCD_D7);
bool isDisplayActive = true; // Tracks whether the screen should be on or off

// --- Function Prototypes ---
float readStableTemperature();

void setup() {
  // 1. Initialize LCD
  lcd.begin(16, 2);
  lcd.print("System Booting..");
  
  // 2. Initialize IR Receiver
  IrReceiver.begin(IR_RECV_PIN);
  
  delay(1500);
  lcd.clear();
}

void loop() {
  // 1. Check for incoming IR signals (Non-blocking mode)
  if (IrReceiver.decode()) {
    if (IrReceiver.decodedIRData.decodedRawData == CMD_POWER_TOGGLE) {
      isDisplayActive = !isDisplayActive; // Toggle display state
      
      if (!isDisplayActive) {
        lcd.clear(); // Clear the screen immediately when turning off
      }
    }
    IrReceiver.resume(); // Flush buffer, ready to receive the next signal
  }

  // 2. Read sensor and update display (only if system is active)
  if (isDisplayActive) {
    float tempCelsius = readStableTemperature();

    lcd.setCursor(0, 0);
    lcd.print("Temp Monitor    "); // Trailing spaces clear leftover characters
    
    lcd.setCursor(0, 1);
    lcd.print("Celsius: ");
    lcd.print(tempCelsius, 1);     // Print with 1 decimal place accuracy
    lcd.print(" C  ");
  }
  
  // Small delay for main loop stability and to prevent LCD flickering
  delay(150); 
}

/**
 * @brief Reads the LM35 sensor multiple times and averages the result 
 * to filter out noise and provide a stable reading.
 * @return float Temperature in degrees Celsius
 */
float readStableTemperature() {
  float sumVoltage = 0;
  const int numSamples = 10;
  
  for (int i = 0; i < numSamples; i++) {
    int rawADC = analogRead(LM35_PIN);
    
    // Convert 10-bit ADC value (0-1023) to Voltage (0-5.0V)
    float voltage = rawADC * (5.0 / 1023.0);
    sumVoltage += voltage;
    delay(10); // Short delay between samples to let the ADC settle
  }
  
  float avgVoltage = sumVoltage / numSamples;
  
  // The LM35 outputs 10mV per degree Celsius (0.01V = 1C)
  // Therefore, Temperature = Voltage / 0.01 = Voltage * 100.0
  return avgVoltage * 100.0; 
}