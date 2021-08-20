/**
 * HD44780 LCD16x2 requires 5V (works on 3V3 too, but contrast is very low)
 * ESP32 is 3V3 logic
 * => level shifters needed for I²C signals
 */

//YWROBOT
//Compatible with the Arduino IDE 1.0
//Library version:1.1
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27,16,2);  // set the LCD address to 0x27 for a 16 chars and 2 line display

void setup()
{
  lcd.init();                      // initialize the lcd 
  // Print a message to the LCD.
  lcd.backlight();
  lcd.setCursor(1,0);
  lcd.print("Hello, world!");
  lcd.setCursor(0,1);
  lcd.print("Ywrobot Arduino!");
  delay(2000);
  lcd.clear();
}


void loop()
{
}