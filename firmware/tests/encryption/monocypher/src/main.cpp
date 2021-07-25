#include <Arduino.h>
#include "monocypher.h"
#include "stdlib.h"
extern "C"
{
#include "bootloader_random.h"
}

void arc4random_buf(void *buf, size_t nbytes)
{
	esp_fill_random(buf, nbytes);
}

void printArray(const char* str, const byte *buf, size_t nbytes)
{
	Serial.print(str);
	for(int i=0;i<nbytes;i++)
	{
		Serial.printf("%02x", buf[i]);
	}
	Serial.println();
}

void setup()
{
	Serial.begin(115200);
	Serial.printf("Build %s\r\n", __TIMESTAMP__);
	bootloader_random_enable();
	uint8_t key[32];						/* Random, secret session key  */
	uint8_t nonce[24];						/* Use only once per key       */
	uint8_t plain_text[12] = "Lorem ipsum"; /* Secret message */
	uint8_t mac[16];						/* Message authentication code */
	uint8_t cipher_text[12];				/* Encrypted message */

	arc4random_buf(key, 32);
	arc4random_buf(nonce, 24);
	printArray("Key:", key, 32);
	printArray("Nonce:", nonce, 24);

	//encrypt
	crypto_lock(mac, cipher_text, key, nonce, plain_text, sizeof(plain_text));
	/* Wipe secrets if they are no longer needed */
	crypto_wipe(plain_text, 12);

	//Transmit cipher_text, nonce, and mac over the network,  store them in a file, etc. 
	printArray("Cipher text:", cipher_text, sizeof(cipher_text));
	printArray("MAC:", mac, sizeof(mac));
	printArray("Wiped plain text: ", plain_text, sizeof(plain_text));

	//decrypt
	if (crypto_unlock(plain_text, key, nonce, mac, cipher_text, 12))
	{
		/* The message is corrupted. 
		* Wipe key if it is no longer needed, 
		* and abort the decryption. 
		*/
		crypto_wipe(key, 32);
	}
	else
	{
		/* ...do something with the decrypted text here... */
		Serial.printf("Decoded plain text:%s\r\n",plain_text);

		/* Finally, wipe secrets if they are no longer needed */
		crypto_wipe(plain_text, 12);
		crypto_wipe(key, 32);
	}
}

void loop()
{
	// put your main code here, to run repeatedly:
}