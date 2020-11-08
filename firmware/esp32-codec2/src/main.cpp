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

void exportEncodedData();
void exportDecodedData();
void encodingSpeed();
void decodingSpeed();

const int mode = CODEC2_MODE_1200;

CODEC2 *codec2;
int nsam, nbit, nbyte;
short *buf;
unsigned char *bits;

void setup()
{
  Serial.begin(115200);

  unsigned long startTime = millis();
  while (!Serial && (startTime + (10 * 1000) > millis()))
  {
  }
  Serial.printf("Build %s\r\n", __TIMESTAMP__);

  codec2 = codec2_create(mode);
  nsam = codec2_samples_per_frame(codec2);
  nbit = codec2_bits_per_frame(codec2);
  buf = (short *)malloc(nsam * sizeof(short));
  nbyte = (nbit + 7) / 8;
  bits = (unsigned char *)malloc(nbyte * sizeof(char));

  codec2_set_natural_or_gray(codec2, 0);

  Serial.printf("Number of audio samples per packet: %d\r\n", nsam);

  //exportEncodedData();
  //exportDecodedData();
  //encodingSpeed();
  decodingSpeed();

  codec2_destroy(codec2);
}

void loop()
{
  Serial.print(".");
  delay(500);
}

/* This function will export the codec2 packet to the serial port.
 * The concatenated packets will be base16-encoded.
 * Copy these lines of hex data and paste them into a file (e.g. lookdave.hex)
 * You can convert that file to a binary format (e.g. lookdave.bit) using xxd:
 *    xxd -r -p lookdave.hex lookdave.bit
 * This bit-file can then be decoded back to raw audio using c2dec.
 */
void exportEncodedData()
{
  int nsam_ctr = 0;
  int bufsize = nsam << 1;
  int linectr = 0;
  Serial.println();
  while (nsam_ctr + bufsize < lookdave_8Khz_raw_len)
  {
    memcpy(buf, lookdave_8Khz_raw + nsam_ctr, bufsize);
    codec2_encode(codec2, bits, buf);
    for (int i = 0; i < nbyte; i++)
    {
      Serial.printf("%02x", bits[i]);
      if ((++linectr) % 30 == 0)
      {
        Serial.println();
      }
    }
    nsam_ctr += bufsize;
  }
  Serial.println();
}

/* This function reads codec2 packets and turns them back to audio.
 * The data are exported as base16-encoded.  Paste the data into a file, e.g. audioout.dat.
 * You can converted that data to raw audio (e.g. audioout.raw) using:
 *    xxd -p -r audioout.dat audioout.raw
 * The raw audio can be converted to a wav-file, which can then be played by about any audioplayer you like:
 *    sox -e signed-integer -b 16 -r 8000 -c 1 audioout.raw audioout.wav
 */
void exportDecodedData()
{
  int nbit_ctr = 0;
  int linectr = 0;
  Serial.println();
  while (nbit_ctr + nbyte < lookdave_bit_len)
  {
    memcpy(bits, lookdave_bit + nbit_ctr, nbyte);
    codec2_decode(codec2, buf, bits);
    for (int i = 0; i < nsam; i++)
    {
      Serial.printf("%02x%02x", buf[i] & 0xFF, (buf[i] >> 8) & 0xFF);
      if ((++linectr) % 30 == 0)
      {
        Serial.println();
      }
    }
    nbit_ctr += nbyte;
  }
  Serial.println();
}

void encodingSpeed()
{
  int nsam_ctr = 0;
  int bufsize = nsam << 1;
  int packet_ctr = 0;
  unsigned long startTime;
  unsigned long totalTime = 0;
  while (nsam_ctr + bufsize < lookdave_8Khz_raw_len)
  {
    memcpy(buf, lookdave_8Khz_raw + nsam_ctr, bufsize);
    startTime = micros();
    codec2_encode(codec2, bits, buf);
    totalTime += micros() - startTime;
    packet_ctr++;
    nsam_ctr += bufsize;
  }
  Serial.printf("Average encoding time per packet: %luµs\r\n", totalTime / packet_ctr);
}

void decodingSpeed()
{
  int nbit_ctr = 0;
  int packet_ctr = 0;
  unsigned long startTime;
  unsigned long totalTime = 0;
  while (nbit_ctr + nbyte < lookdave_bit_len)
  {
    memcpy(bits, lookdave_bit + nbit_ctr, nbyte);
    startTime = micros();
    codec2_decode(codec2, buf, bits);
    totalTime += micros() - startTime;
    packet_ctr++;
    nbit_ctr += nbyte;
  }
  Serial.printf("Average decoding time per packet: %luµs\r\n", totalTime / packet_ctr);
}
