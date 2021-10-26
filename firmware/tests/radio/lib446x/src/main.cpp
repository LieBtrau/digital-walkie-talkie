#include <Arduino.h>
#include "SPI.h"
#include "si446x.h"
#include "radio.h"
#include "modem_configs.h"
#include "error.h"
#include "pkt_buf.h"
#include "AsyncDelay.h"

#define MAX_PAYLOAD_LEN 255
#define OSC_TYPE OPT_TCXO
#define NSEL_PIN 5
#define SDN_PIN 16
#define INT_PIN 4
#define FLAG_CRC_CHECK 0x0001
#define FLAG_WHITEN 0x0002
#define FLAG_MOD_MASK 0x000C
#define FLAG_MOD_GFSK 0x0000
#define FLAG_MOD_FSK 0x0004
#define FLAG_MOD_CW 0x0008

const int MODE_SELECT_PIN = 27;
AsyncDelay delay_3s;
uint16_t flags = FLAG_MOD_GFSK | FLAG_CRC_CHECK;
uint8_t modem_config = DATA_10K_DEV_2K5;
volatile bool radio_irq = true;
struct si446x_device dev;
uint8_t buf[255];
uint8_t tx_backing_buf[16384] = {0};
bool STATUS_TXBUSY = false;
struct pkt_buf tx_queue;
bool isClient = false;

void tx_cb(struct si446x_device *dev, int err);
void rx_cb(struct si446x_device *dev, int err, int len, uint8_t *data);
int reset_si446x();

#if (ESP8266 || ESP32)
ICACHE_RAM_ATTR void onIrqFalling()
#else
void Si446x::onIrqFalling()
#endif
{
	radio_irq = true;
}

int reload_config()
{
	int err;

	err = set_frequency(432500000L);
	if (err)
	{
		return error(err, __FILE__, __LINE__);
	}

	// err = set_dac_output(TCXO_CHAN, settings.tcxo_vpull);
	// if (err) {
	//     return error(err, __FILE__, __LINE__);
	// }

	err = si446x_check_crc(&dev, flags & FLAG_CRC_CHECK);
	if (err)
	{
		return error(err, __FILE__, __LINE__);
	}

	err = si446x_data_whitening(&dev, flags & FLAG_WHITEN);
	if (err)
	{
		return error(err, __FILE__, __LINE__);
	}

	err = set_modem_config(modem_config);
	if (err)
	{
		return error(err, __FILE__, __LINE__);
	}

	if ((flags & FLAG_MOD_MASK) == FLAG_MOD_CW)
	{
		err = si446x_set_mod_type(&dev, MOD_TYPE_CW);
	}
	else if ((flags & FLAG_MOD_MASK) == FLAG_MOD_FSK)
	{
		err = si446x_set_mod_type(&dev, MOD_TYPE_2FSK);
	}
	else if ((flags & FLAG_MOD_MASK) == FLAG_MOD_GFSK)
	{
		err = si446x_set_mod_type(&dev, MOD_TYPE_2GFSK);
	}
	else
	{
		err = si446x_set_mod_type(&dev, MOD_TYPE_2GFSK);
	}

	if (err)
	{
		return error(err, __FILE__, __LINE__);
	}

	return 0;
}

int config_si446x()
{
	int err;

	err = reload_config();

	err = si446x_config_crc(&dev, CRC_SEED_1 | CRC_CCIT_16);
	if (err)
	{
		return error(err, __FILE__, __LINE__);
	}

	//err = si446x_set_tx_pwr(&dev, 0x14);
	err = si446x_set_tx_pwr(&dev, 0x7f);
	if (err)
	{
		return error(err, __FILE__, __LINE__);
	}

	err = si446x_cfg_gpio(&dev, GPIO_SYNC_WORD_DETECT, GPIO_RX_STATE, GPIO_TX_DATA_CLK, GPIO_TX_STATE);
	if (err)
	{
		return error(err, __FILE__, __LINE__);
	}

	return ESUCCESS;
}

int reset_si446x()
{

	int err;

	// Disable pin interrupts while we reset
	//disable_pin_interrupt(GPIO0);
	detachInterrupt(digitalPinToInterrupt(INT_PIN));

	err = si446x_reinit(&dev);

	if (err)
	{
		return error(err, __FILE__, __LINE__);
	}

	err = config_si446x();
	if (err)
	{
		return error(err, __FILE__, __LINE__);
	}

	//enable_pin_interrupt(GPIO0, RISING);
	attachInterrupt(digitalPinToInterrupt(INT_PIN), onIrqFalling, FALLING);

	return ESUCCESS;
}

int send_w_retry(int len, uint8_t *buf)
{
	int attempts;
	int err;

	si446x_idle(&dev);
	//pre_transmit();

	for (attempts = 0; attempts < 3; attempts++)
	{

		err = si446x_send_async(&dev, len, buf, tx_cb);

		if (err == -ERESETSI)
		{

			err = reset_si446x();
			if (err)
			{
				return error(err, __FILE__, __LINE__);
			}

			// Retry
			printf("Si446x BUG: Resetting...\n");
		}
		else if (err)
		{
			return error(err, __FILE__, __LINE__);
		}
		else
		{
			return ESUCCESS;
		}
	}

	// Give up
	return -ETIMEOUT;
}

void tx_cb(struct si446x_device *dev, int err)
{
	if (err)
	{
		error(-err, __FILE__, __LINE__);
	}

	// wdt_feed();

	if (pkt_buf_depth(&tx_queue) > 0)
	{
		int pkt_len = MAX_PAYLOAD_LEN;
		err = pkt_buf_dequeue(&tx_queue, &pkt_len, buf);

		if (err)
		{
			error(-err, __FILE__, __LINE__);
			//post_transmit();
			STATUS_TXBUSY = false;
			si446x_recv_async(dev, 255, buf, rx_cb);
			return;
		}

		//pa_power_keepalive();
		err = send_w_retry(pkt_len, buf);

		if (err)
		{
			error(-err, __FILE__, __LINE__);
			//post_transmit();
			STATUS_TXBUSY = false;
			si446x_recv_async(dev, 255, buf, rx_cb);
			return;
		}
	}
	else
	{
		//post_transmit();
		STATUS_TXBUSY = false;
		si446x_recv_async(dev, 255, buf, rx_cb);
	}
}

void reply(uint8_t cmd, int len, uint8_t *payload)
{
	for (int i = 0; i < len; i++)
	{
		if (i % 16 == 0)
		{
			printf("\r\n");
		}
		printf("%02x ", payload[i]);
	}
	printf("\r\n");
}

void rx_cb(struct si446x_device *dev, int err, int len, uint8_t *data)
{

	// Stop rx timeout timer
	// TA1CCTL0 = 0;                           // TACCR0 interrupt disabled
	// TA1CTL |= TACLR;                        // Stop timer

	if (err)
	{
		STATUS_TXBUSY = false;
		// internal_error((uint8_t) -err);
		si446x_recv_async(dev, 255, buf, rx_cb);
		return;
	}

	// if (len == 4 && data[0] == 'P' && data[1] == 'I' && data[2] == 'N' && data[3] == 'G')
	// {
	// 	uint8_t resp[] = {
	// 		'P',
	// 		'O',
	// 		'N',
	// 		'G',
	// 	};
	// 	STATUS_TXBUSY = true;
	// 	// gpio_write(TX_ACT_PIN, HIGH);
	// 	// err = set_gate_bias(settings.tx_gate_bias);
	// 	si446x_setup_tx(dev, sizeof(resp), resp, tx_cb);
	// 	if (si446x_fire_tx(dev) == -ERESETSI)
	// 	{
	// 		reset_si446x();
	// 		si446x_recv_async(dev, 255, buf, rx_cb);
	// 	}
	// 	return;
	// }
	// else
	// {
	// 	// Handle RX'd packet
		reply(0, len, data);
	// }
	STATUS_TXBUSY = false;
	si446x_recv_async(dev, 255, buf, rx_cb);
}

void setup()
{
	pkt_buf_init(&tx_queue, sizeof(tx_backing_buf), tx_backing_buf);
	delay_3s.start(3000, AsyncDelay::MILLIS);

	pinMode(MODE_SELECT_PIN, INPUT_PULLUP);
	isClient = digitalRead(MODE_SELECT_PIN) == HIGH ? true : false;

	// put your setup code here, to run once:
	si446x_create(&dev, NSEL_PIN, SDN_PIN, INT_PIN, XTAL_FREQ, OSC_TYPE);

	int err = si446x_init(&dev);
	if (err)
	{
		error(-err, __FILE__, __LINE__);
		return;
	}

	config_si446x();

	printf("Successfully initialized Si446x!\n");

	attachInterrupt(digitalPinToInterrupt(INT_PIN), onIrqFalling, FALLING);

	err = si446x_recv_async(&dev, 255, buf, rx_cb);

	if (err)
	{
		error(-err, __FILE__, __LINE__);
		return;
	}
	printf("Setup ok\n");
}

void loop()
{
	if (radio_irq)
	{
		radio_irq = false;
		int err = si446x_update(&dev);
		if (err)
		{
			printf("Err: %d", err);
			// TODO: Better error handling
			si446x_reinit(&dev);
			config_si446x();
			//si446x_recv_async(&dev, 255, buf, rx_cb);
			error(-err, __FILE__, __LINE__);
		}
	}
	if (delay_3s.isExpired() && isClient)
	{
		delay_3s.repeat(); // Count from when the delay expired, not now
		byte testbuf[160];
		for(int i=0;i<sizeof(testbuf);i++)
		{
			testbuf[i]=i;
		}
		printf("send something %d\r\n", send_w_retry(sizeof(testbuf), testbuf));
	}
}