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

void authenticatedSymmetricEncryption()
{
	const unsigned char *MESSAGE = (const unsigned char *)"test";
	size_t MESSAGE_LEN = strlen((const char *)MESSAGE); //4
	size_t CIPHERTEXT_LEN = crypto_secretbox_MACBYTES + MESSAGE_LEN;
	unsigned char key[crypto_secretbox_KEYBYTES];

	unsigned char ciphertext[CIPHERTEXT_LEN];

	//Generate nonce
	unsigned char nonce[crypto_secretbox_NONCEBYTES];
	randombytes_buf(nonce, sizeof nonce);

	//Generate key
	crypto_secretbox_keygen(key);

	//Encryption
	//The trailing zero of the string doesn't get encrypted.
	crypto_secretbox_easy(ciphertext, MESSAGE, MESSAGE_LEN, nonce, key);

	printArray("key:", key, crypto_secretbox_KEYBYTES);
	printArray("nonce:", nonce, crypto_secretbox_NONCEBYTES);
	//Decryption
	unsigned char decrypted[MESSAGE_LEN + 1]; //add room for a trailing zero.
	if (crypto_secretbox_open_easy(decrypted, ciphertext, CIPHERTEXT_LEN, nonce, key) != 0)
	{
		/* message forged! */
		Serial.println("message forged");
	}
	Serial.println((const char *)decrypted);

	/**
	 * If another encryption is needed, then use another nonce.  Simple solution: nonce++
	 * This makes is possible to avoid having to send the complete nonce for every encryption.
	 * We can suffice by sending the offset from the original value.  That will take less bits.
	 */
	sodium_increment(nonce, sizeof nonce);

	//Clear sensitive data
	sodium_memzero(key, crypto_secretbox_KEYBYTES);
}

/**
 * Remark that the client and the server execute different functions for calculating the shared keys.  This
 * is because Sodium implements key exchange using a hash function.
 * The key exchange function will also return true when the session keys are not equal in client and server.
 */
void keyExchange()
{
	//Assymmetric keys for key exchange
	unsigned char client_pk[crypto_kx_PUBLICKEYBYTES], client_sk[crypto_kx_SECRETKEYBYTES];
	unsigned char server_pk[crypto_kx_PUBLICKEYBYTES], server_sk[crypto_kx_SECRETKEYBYTES];
	//Shared secret material after key exchange
	unsigned char server_rx[crypto_kx_SESSIONKEYBYTES], server_tx[crypto_kx_SESSIONKEYBYTES];
	unsigned char client_rx[crypto_kx_SESSIONKEYBYTES], client_tx[crypto_kx_SESSIONKEYBYTES];

	/* Generate the client's key pair */
	crypto_kx_keypair(client_pk, client_sk);
	/* Generate the server's key pair */
	crypto_kx_keypair(server_pk, server_sk);

	//Client sends client_pk to server
	//Server sends server_pk to client

	/* Client Computes two shared keys using the server's public key and the client's secret key.
   	client_rx will be used by the client to receive data from the server,
   	client_tx will by used by the client to send data to the server. */
	if (crypto_kx_client_session_keys(client_rx, client_tx, client_pk, client_sk, server_pk) != 0)
	{
		/* Suspicious server public key, bail out */
		Serial.println("Client: Server public key error");
		return;
	}
	printArray("Client rx: ", client_rx, crypto_kx_SESSIONKEYBYTES);
	printArray("Client tx: ", client_tx, crypto_kx_SESSIONKEYBYTES);

	/* Server computes two shared keys using the client's public key and the server's secret key.
   	server_rx will be used by the server to receive data from the client,
   	server_tx will by used by the server to send data to the client. */
	if (crypto_kx_server_session_keys(server_rx, server_tx, server_pk, server_sk, client_pk) != 0)
	{
		/* Suspicious client public key, bail out */
		Serial.println("Server: client public key error");
		return;
	}
	printArray("Server rx: ", server_rx, crypto_kx_SESSIONKEYBYTES);
	printArray("Server tx: ", server_tx, crypto_kx_SESSIONKEYBYTES);
	Serial.println("All ok");
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
	authenticatedSymmetricEncryption();
	keyExchange();
	Serial.println("setup done.");
}

void loop()
{
	// put your main code here, to run repeatedly:
}