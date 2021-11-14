#include <Arduino.h>
#include "BluetoothSerial.h"
#include "KissTnc.h"

BluetoothSerial SerialBT;
KissTnc kisstnc(&SerialBT);
byte dataBuffer[500];

void exitKissHandler()
{
	Serial.println("Host wants to exit kiss-mode.");
}

void errorHandler(int err, const char *file, int line)
{
	Serial.printf("Error: (%d) at %s:%d\n\r", err, file, line);
}

void dataReceivedHandler(int length)
{
	kisstnc.readBytes(dataBuffer, length);
	Serial.printf("RX: %d bytes\r\n", length);
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