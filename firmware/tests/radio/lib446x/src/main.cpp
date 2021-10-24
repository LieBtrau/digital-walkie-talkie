#include <Arduino.h>
#include "SPI.h"
#include "si446x.h"
#include "radio.h"
#include "modem_configs.h"
#include "error.h"

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
uint16_t flags = FLAG_MOD_GFSK | FLAG_CRC_CHECK;
uint8_t modem_config = DATA_10K_DEV_2K5;
volatile bool radio_irq = true;
struct si446x_device dev;

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
		return err;
	}

	// err = set_dac_output(TCXO_CHAN, settings.tcxo_vpull);
	// if (err) {
	//     return err;
	// }

	err = si446x_check_crc(&dev, flags & FLAG_CRC_CHECK);
	if (err)
	{
		return err;
	}

	err = si446x_data_whitening(&dev, flags & FLAG_WHITEN);
	if (err)
	{
		return err;
	}

	err = set_modem_config(modem_config);
	if (err)
	{
		return err;
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
		return err;
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
		return err;
	}

	//err = si446x_set_tx_pwr(&dev, 0x14);
	err = si446x_set_tx_pwr(&dev, 0x7f);
	if (err)
	{
		return err;
	}

	err = si446x_cfg_gpio(&dev, GPIO_SYNC_WORD_DETECT, GPIO_RX_STATE, GPIO_TX_DATA_CLK, GPIO_TX_STATE);
	if (err)
	{
		return err;
	}

	return ESUCCESS;
}

void setup()
{
	// put your setup code here, to run once:
	si446x_create(&dev, NSEL_PIN, SDN_PIN, INT_PIN, XTAL_FREQ, OSC_TYPE);

	int err = si446x_init(&dev);
	if (err)
	{
		error(-err, __FILE__, __LINE__);
	}

	config_si446x();

	printf("Successfully initialized Si446x!\n");

	attachInterrupt(digitalPinToInterrupt(INT_PIN), onIrqFalling, FALLING);
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
}