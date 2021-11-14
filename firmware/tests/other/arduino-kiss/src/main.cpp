#include <Arduino.h>
#include "BluetoothSerial.h"
#include "KissTnc.h"

BluetoothSerial SerialBT;
KissTnc kisstnc(&SerialBT);
byte dataBuffer[500];
long rxDataCounter = 0;

void exitKissHandler()
{
	Serial.println("Host wants to exit kiss-mode.");
}

void errorHandler(int err, const char *file, int line)
{
	Serial.printf("Error: (%d) at %s:%d\n\r", err, file, line);
}

/**
 * @brief Print codec2 encoded packet to serial port in hex-format.
 * 1. You can paste this output in a file named: audio.hex
 * 2. Then convert that file to raw bits using : xxd -r -p audio.hex audio.bit
 * 3. Then convert that raw bit file to raw audio using (e.g. for codec2-1200): c2dec 1200 audio.bit audio.raw
 * 4. Then convert the raw audio file to wav using : sox -e signed-integer -b 16 -r 8000 -c 1 audio.raw audio.wav
 * @param length 
 */
void dataReceivedHandler(int length)
{
	kisstnc.readBytes(dataBuffer, length);
	for (int i = 0; i < length; i++)
	{
		Serial.printf("%02x", dataBuffer[i]);
		if ((++rxDataCounter) % 30 == 0)
		{
			Serial.println();
		}
	}
}

void setHardwareHandler(int length)
{
	kisstnc.readBytes(dataBuffer, length);
	Serial.printf("Set hardware: %d bytes\r\n", length);
}

void parameterUpdateHandler(byte txdelay, byte p, byte slotTime, byte txTail, byte fullDuplex)
{
	Serial.printf("TxDelay: %d\tP: %d\tSlotTime: %d\tTxTail: %d\tFullDuplex: %d\r\n", txdelay, p, slotTime, txTail, fullDuplex);
}

void setup()
{
	Serial.begin(115200);
	SerialBT.begin("ESP32 KISS TNC");
	kisstnc.onExitKiss(exitKissHandler);
	kisstnc.onError(errorHandler);
	kisstnc.onDataReceived(dataReceivedHandler);
	kisstnc.onSetHardwareReceived(setHardwareHandler);
	kisstnc.onRadioParameterUpdate(parameterUpdateHandler);
	delay(1000);
	Serial.printf("Build %s\r\n", __TIMESTAMP__);
}

void loop()
{
	kisstnc.loop();
}