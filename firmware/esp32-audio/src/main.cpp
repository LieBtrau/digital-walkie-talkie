/*
 * Test to see how well codec2 performs on ESP32.
 * To solve the stack overflow problem: 
 *    triple stack size from 8192 to 3*8192: ~/.platformio/packages/framework-arduinoespressif32/cores/esp32/main.cpp
 *  * 
 * https://www.xtronical.com/basics/audio/dacs-for-sound/playing-wav-files/ DACaudio for ESP32
 */

#include "Arduino.h"
#include "codec2.h"
#include "lookdave.h"

int16_t *p16bitdata;

const int mode = CODEC2_MODE_1200;

CODEC2 *codec2;
int nsam, nbit, nbyte;
short *buf;
unsigned char *bits;

void setup()
{
  Serial.begin(115200);
  Serial.println("started");

  unsigned long startTime = millis();
  while (!Serial && (startTime + (10 * 1000) > millis()))
  {
  }

  codec2 = codec2_create(mode);
  nsam = codec2_samples_per_frame(codec2);
  nbit = codec2_bits_per_frame(codec2);
  buf = (short *)malloc(nsam * sizeof(short));
  nbyte = (nbit + 7) / 8;
  bits = (unsigned char *)malloc(nbyte * sizeof(char));

  codec2_set_natural_or_gray(codec2, 0);

  char s[100];
  sprintf(s, "Number of audio samples: %d", nsam);
  Serial.println(s);

  int i=0;
  int bufsize=nsam<<1;
  lookdave_8Khz_raw_len=1100;
  while(i+bufsize<lookdave_8Khz_raw_len)
  {
    memcpy(buf, lookdave_8Khz_raw + i, bufsize);
    codec2_encode(codec2, bits, buf);
    codec2_decode(codec2, buf, bits);
    for (int x = 0; x < nbyte; x++)
    {
      sprintf(s, "%02x ", bits[x]);
      Serial.print(s);
    }
    Serial.println();
    i+=bufsize;
  }
  // char s[100];

  // uint8_t c2Buf[nbyte];
  // int16_t audioOut[nsam];

  // unsigned long ulTimer = millis();
  // 
  // sprintf(s, "Encoding takes %d ms.", (int)(millis() - ulTimer));
  // Serial.println(s);
  // sprintf(s, "Encoding should take less than %d ms.", nsam / 8);
  // Serial.println(s);
  // ulTimer = millis();
  // codec2_decode(codec2, audioOut, c2Buf);
  // sprintf(s, "Decoding takes %d ms.", (int)(millis() - ulTimer));
  // Serial.println(s);
  // codec2_destroy(codec2);
  // for (int x = 0; x < nsam; x++)
  // {
  //   sprintf(s, "%d %d", audioBuf[x], audioOut[x]);
  //   Serial.println(s);
  // }
}

void loop()
{
  Serial.print(".");
  delay(500);
}