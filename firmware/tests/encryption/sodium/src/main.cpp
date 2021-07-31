#include <Arduino.h>
#include <sodium.h>

extern "C"
{
#include "bootloader_random.h"
}


void printArray(const char *str, const byte *buf, size_t nbytes)
{
	Serial.print(str);
	for (int i = 0; i < nbytes; i++)
	{
		Serial.printf("%02x", buf[i]);
	}
	Serial.println();
}

void setup()
{
	Serial.begin(115200);
	Serial.printf("\r\nBuild %s\r\n", __TIMESTAMP__);
	bootloader_random_enable();
	if (sodium_init() < 0)
	{
		/* panic! the library couldn't be initialized, it is not safe to use */
		return;
	}
	const unsigned char * MESSAGE  = (const unsigned char *)"test";
	size_t MESSAGE_LEN = strlen((const char*)MESSAGE);//4
	size_t CIPHERTEXT_LEN = crypto_secretbox_MACBYTES + MESSAGE_LEN;
	Serial.println(MESSAGE_LEN, DEC);
	unsigned char key[crypto_secretbox_KEYBYTES];
	unsigned char nonce[crypto_secretbox_NONCEBYTES];
	unsigned char ciphertext[CIPHERTEXT_LEN];

	//Generate key & nonce
	crypto_secretbox_keygen(key);
	randombytes_buf(nonce, sizeof nonce);
	//Encryption
	//The trailing zero of the string doesn't get encrypted.
	crypto_secretbox_easy(ciphertext, MESSAGE, MESSAGE_LEN, nonce, key);

	printArray("key:", key, crypto_secretbox_KEYBYTES);
	printArray("nonce:", nonce, crypto_secretbox_NONCEBYTES);
	//Decryption
	unsigned char decrypted[MESSAGE_LEN+1];	//add room for a trailing zero.
	if (crypto_secretbox_open_easy(decrypted, ciphertext, CIPHERTEXT_LEN, nonce, key) != 0)
	{
		/* message forged! */
		Serial.println("message forged");
	}
	//Clear sensitive data
	sodium_memzero(key, crypto_secretbox_KEYBYTES);
	Serial.println((const char*)decrypted);
	Serial.println("setup done.");
}

void loop()
{
	// put your main code here, to run repeatedly:
}