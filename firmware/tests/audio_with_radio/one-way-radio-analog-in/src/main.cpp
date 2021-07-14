#include <Arduino.h>
#include "lookdave.h"
#include "Codec2Interface.h"
#include "Codec2Decoder.h"
#include "Codec2Encoder.h"
#include "Sgtl5000_Output.h"
#include "Sgtl5000_Input.h"
#include "control_sgtl5000.h"
#include "pinconfig.h"
#include "RadioInterface.h"
#include "Bounce2.h"

const int PACKET_INTERVAL_ms = 80; //in ms
const int PACKET_SIZE = 20;

Codec2Interface c2i;
Codec2Decoder c2d(&c2i);
Codec2Encoder c2e(&c2i);
Sgtl5000_Output *output;
Sgtl5000_Input *input;
AudioControlSGTL5000 audioShield;
RadioInterface ri(PIN_CS, PIN_IRQ, PIN_SDN);
static i2s_pin_config_t i2s_pin_config = {
	.bck_io_num = PIN_BCLK,	   // Serial Clock (SCK)
	.ws_io_num = PIN_LRCLK,	   // Word Select (WS)
	.data_out_num = PIN_SDOUT, // data out to audio codec
	.data_in_num = PIN_SDIN	   // data from audio codec
};
uint8_t data[PACKET_SIZE];
int nsam_ctr;
short *buf;
Bounce pttSwitch = Bounce();
typedef enum
{
	LISTENING,
	SPEAKING
} wtState;
wtState wtstate = LISTENING;

void setup()
{
	Serial.begin(115200);
	pinMode(LED_BUILTIN, OUTPUT);
	unsigned long startTime = millis();
	while (!Serial && (startTime + (10 * 1000) > millis()))
	{
	}
	Serial.printf("Build %s\r\n", __TIMESTAMP__);
	if (!c2d.init() || !c2e.init())
	{
		while (true)
		{
		}
	}
	buf = (short *)malloc(c2i.getAudioSampleCount() * sizeof(short));
	Serial.println("Starting I2S Output");
	pttSwitch.attach(PIN_MODE_SELECT, INPUT_PULLUP);
	output = new Sgtl5000_Output(I2S_NUM_0, &i2s_pin_config);
	input = new Sgtl5000_Input(I2S_NUM_0, &i2s_pin_config);
	output->start(&c2d); //init needed here to generate MCLK, needed for SGTL5000 init.
	Serial.printf("SGTL5000 %s initialized.\r\n", audioShield.enable() ? "is" : "not");
	audioShield.volume(0.5);
	audioShield.lineInLevel(2); //2.22Vpp equals maximum output.
	if (!ri.init())
	{
		Serial.println("Can't init radio");
		while (true)
			;
	}
	Serial.println("Ready to roll");
}

int txstate = 0;
void sendPacket()
{
	switch (txstate)
	{
	case 0:
		if (c2i.isEncodedFrameAvailable() && c2i.getEncodedAudio(data))
		{
			txstate = 1;
		}
		break;
	case 1:
		if (c2i.isEncodedFrameAvailable() && c2i.getEncodedAudio(data + c2i.getCodec2PacketSize()))
		{
			txstate = 2;
		}
		break;
	case 2:
		ri.sendPacket(data);
		txstate = 0;
		break;
	default:
		txstate = 0;
		break;
	}
}

int rxstate = 0;
void receivePacket()
{
	switch (rxstate)
	{
	case 0:
		if (ri.receivePacket(data))
		{
			rxstate = 1;
		}
		break;
	case 1:
		if (c2i.isDecodingInputBufferSpaceLeft() && c2i.startDecodingAudio(data))
		{
			rxstate = 2;
		}
		break;
	case 2:
		if (c2i.isDecodingInputBufferSpaceLeft() && c2i.startDecodingAudio(data + c2i.getCodec2PacketSize()))
		{
			rxstate = 0;
		}
		break;
	default:
		rxstate = 0;
		break;
	}
}

void loop()
{
	pttSwitch.update();
	switch (wtstate)
	{
	case LISTENING:
		receivePacket();
		if (pttSwitch.read()==LOW)
		{
			//PTT-pushed
			Serial.println("PTT-pushed");
			output->stop();
			vTaskDelay(1);
			input->start(&c2e);
			wtstate = SPEAKING;
		}
		break;
	case SPEAKING:
		sendPacket();
		if (pttSwitch.read()==HIGH)
		{
			//PTT-released
			Serial.println("PTT-released");
			input->stop();
			vTaskDelay(1);
			output->start(&c2d);
			wtstate = LISTENING;
		}
		break;
	default:
		wtstate = LISTENING;
		break;
	}
	vTaskDelay(1);
}