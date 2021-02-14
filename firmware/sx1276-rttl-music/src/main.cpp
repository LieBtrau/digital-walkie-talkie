/*
  RadioLib SX127x FSK Modem Example : Direct mode : playing some RTTTL music using FSK on  434MHz.  
  It works well on the RSP1A with CubicSDR.
  SX1278 generates a 48.1kHz clock on DIO1, the ESP32 sends its "data" to SX1278.DIO2.


   For default module settings, see the wiki page
   https://github.com/jgromes/RadioLib/wiki/Default-configuration#sx127xrfm9x---fsk-modem

   For full API reference, see the GitHub Pages
   https://jgromes.github.io/RadioLib/

*/

// include the library
#include <RadioLib.h>
#include <Tone32.h>
#include <NonBlockingRtttl.h>

//project's contants
const char *tetris = "tetris:d=4,o=5,b=160:e6,8b,8c6,8d6,16e6,16d6,8c6,8b,a,8a,8c6,e6,8d6,8c6,b,8b,8c6,d6,e6,c6,a,2a,8p,d6,8f6,a6,8g6,8f6,e6,8e6,8c6,e6,8d6,8c6,b,8b,8c6,d6,e6,c6,a,a";
const char *arkanoid = "Arkanoid:d=4,o=5,b=140:8g6,16p,16g.6,2a#6,32p,8a6,8g6,8f6,8a6,2g6";
const char *mario = "mario:d=4,o=5,b=100:16e6,16e6,32p,8e6,16c6,8e6,8g6,8p,8g,8p,8c6,16p,8g,16p,8e,16p,8a,8b,16a#,8a,16g.,16e6,16g6,8a6,16f6,8g6,8e6,16c6,16d6,8b,16p,8c6,16p,8g,16p,8e,16p,8a,8b,16a#,8a,16g.,16e6,16g6,8a6,16f6,8g6,8e6,16c6,16d6,8b,8p,16g6,16f#6,16f6,16d#6,16p,16e6,16p,16g#,16a,16c6,16p,16a,16c6,16d6,8p,16g6,16f#6,16f6,16d#6,16p,16e6,16p,16c7,16p,16c7,16c7,p,16g6,16f#6,16f6,16d#6,16p,16e6,16p,16g#,16a,16c6,16p,16a,16c6,16d6,8p,16d#6,8p,16d6,8p,16c6";
byte songIndex = 0; //which song to play when the previous one finishes

// SX1278 has the following connections:
// 3V3        ESP32.3V3
// GND        ESP32.GND
// MOSI pin:  ESP32.23
// MISO pin:  ESP32.19
// SCK pin :  ESP32.18
// NSS pin:   ESP32.5
// RESET pin: ESP32.36
// DIO0 pin:  ESP32.39
// DIO1 pin:  ESP32.34  //Clock pin in continuous mode
// DIO2 pin:  ESP32.16  //Data pin in continuous mode
SX1278 radio = new Module(5, 39, 36, 34);
const int BUZZER_PIN = 16;
const int BUZZER_CHANNEL = 0;

// or using RadioShield
// https://github.com/jgromes/RadioShield
//SX1278 fsk = RadioShield.ModuleA;

void setup()
{
  Serial.begin(115200);
  // initialize SX1278 FSK modem with default settings
  Serial.print(F("[SX1278] Initializing ... "));
  int state = radio.beginFSK();
  if (state == ERR_NONE)
  {
    Serial.println(F("success!"));
  }
  else
  {
    Serial.print(F("failed, code "));
    Serial.println(state);
    while (true)
      ;
  }
  state = radio.setDataShaping(0.0);
  if (state != ERR_NONE)
  {
    Serial.println(F("[SX1278] Unable to set data shaping, code "));
    Serial.println(state);
    while (true)
      ;
  }
  state = radio.transmitDirect(); //default 434.000MHz
  if (state != ERR_NONE)
  {
    Serial.println(F("[SX1278] Unable to start direct transmission mode, code "));
    Serial.println(state);
    while (true)
      ;
  }

  Serial.println(F("[SX1278] Setup done. "));
}

void loop()
{
  if (!rtttl::isPlaying())
  {
    delay(1000);
    if (songIndex == 0)
    {
      rtttl::begin(BUZZER_PIN, mario);
      songIndex++; //ready for next song
    }
    else if (songIndex == 1)
    {
      rtttl::begin(BUZZER_PIN, arkanoid);
      songIndex++; //ready for next song
    }
    else if (songIndex == 2)
    {
      rtttl::begin(BUZZER_PIN, tetris);
      songIndex = 0; //ready for next song
    }
  }
  else
  {
    rtttl::play();
  }
}