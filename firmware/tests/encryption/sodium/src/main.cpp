#include <Arduino.h>
#include "micro-tls.h"

extern "C"
{
#include "bootloader_random.h"
}

void printArray(const char *str, const uint8_t *buf, size_t nbytes)
{
	Serial.print(str);
	for (int i = 0; i < nbytes; i++)
	{
		Serial.printf("%02x", buf[i]);
	}
	Serial.println();
}

// void authenticatedSymmetricEncryption()
// {
// 	const unsigned char *MESSAGE = (const unsigned char *)"test";
// 	size_t MESSAGE_LEN = strlen((const char *)MESSAGE); //4
// 	size_t CIPHERTEXT_LEN = crypto_secretbox_MACBYTES + MESSAGE_LEN;
// 	unsigned char key[crypto_secretbox_KEYBYTES];

// 	unsigned char ciphertext[CIPHERTEXT_LEN];

// 	//Generate nonce
// 	unsigned char nonce[crypto_secretbox_NONCEBYTES];
// 	randombytes_buf(nonce, sizeof nonce);

// 	//Generate key
// 	crypto_secretbox_keygen(key);

// 	//Encryption
// 	//The trailing zero of the string doesn't get encrypted.
// 	crypto_secretbox_easy(ciphertext, MESSAGE, MESSAGE_LEN, nonce, key);

// 	printArray("key:", key, crypto_secretbox_KEYBYTES);
// 	printArray("nonce:", nonce, crypto_secretbox_NONCEBYTES);
// 	//Decryption
// 	unsigned char decrypted[MESSAGE_LEN + 1]; //add room for a trailing zero.
// 	if (crypto_secretbox_open_easy(decrypted, ciphertext, CIPHERTEXT_LEN, nonce, key) != 0)
// 	{
// 		/* message forged! */
// 		Serial.println("message forged");
// 	}
// 	Serial.println((const char *)decrypted);

// 	/**
// 	 * If another encryption is needed, then use another nonce.  Simple solution: nonce++
// 	 * This makes is possible to avoid having to send the complete nonce for every encryption.
// 	 * We can suffice by sending the offset from the original value.  That will take less bits.
// 	 */
// 	sodium_increment(nonce, sizeof nonce);

// 	//Clear sensitive data
// 	sodium_memzero(key, crypto_secretbox_KEYBYTES);
// }

uint32_t getEsp32UniqueId()
{
	uint32_t chipId = 0;
	for (int i = 0; i < 17; i = i + 8)
	{
		chipId |= ((ESP.getEfuseMac() >> (40 - i)) & 0xff) << i;
	}
	return chipId;
}


// void micro_tls()
// {
// 	//Client preparation
// 	unsigned char client_random[32];
// 	unsigned char client_pk[crypto_kx_PUBLICKEYBYTES], client_sk[crypto_kx_SECRETKEYBYTES];

	

// 	//ClientHello : client_random & client_pk to server

// 	//Server preparation
// 	unsigned char server_random[32];
// 	unsigned char server_pk[crypto_kx_PUBLICKEYBYTES], server_sk[crypto_kx_SECRETKEYBYTES];

// 	randombytes_buf(server_random, sizeof server_random);
// 	crypto_kx_keypair(server_pk, server_sk);

// 	//Server calculates "Handshake secret"
// 	unsigned char server_rx[crypto_kx_SESSIONKEYBYTES], server_tx[crypto_kx_SESSIONKEYBYTES];
// 	if (crypto_kx_server_session_keys(server_rx, server_tx, server_pk, server_sk, client_pk) != 0)
// 	{
// 		/* Suspicious client public key, bail out */
// 		Serial.println("Server: client public key error");
// 		return;
// 	}

// 	//Calculate hash of ClientHello|ServerHello
// 	unsigned char hash[crypto_generichash_BYTES];
// 	crypto_generichash_state state;
// 	crypto_generichash_init(&state, server_rx, crypto_kx_SESSIONKEYBYTES, sizeof hash);
// 	crypto_generichash_update(&state, client_random, 32);
// 	crypto_generichash_update(&state, client_pk, crypto_kx_PUBLICKEYBYTES);
// 	crypto_generichash_update(&state, server_random, 32);
// 	crypto_generichash_update(&state, server_pk, crypto_kx_PUBLICKEYBYTES);
// 	crypto_generichash_final(&state, hash, sizeof hash);
// 	//Derive traffic secrets
// 	unsigned char server_handshake_iv[crypto_secretbox_NONCEBYTES];
// 	unsigned char server_handshake_key[crypto_secretbox_KEYBYTES];

// 	//crypto_kdf_KEYBYTES equal to crypto_generichash_BYTES ?
// 	crypto_kdf_derive_from_key(server_handshake_iv, sizeof server_handshake_iv, 1, "key_s_trf", hash);
// 	crypto_kdf_derive_from_key(server_handshake_key, sizeof server_handshake_key, 1, "iv_s__trf", hash);
// }

union mix_t
{
    uint32_t theDWord;
    uint8_t theBytes[4];
};

void setup()
{
	Serial.begin(115200);
	Serial.printf("\r\nBuild %s\r\n", __TIMESTAMP__);
	bootloader_random_enable();
	Serial.printf("Chip ID: 0x%08x\r\n", getEsp32UniqueId());

	mix_t client_id, server_id;
	client_id.theDWord = getEsp32UniqueId();
	server_id.theDWord = getEsp32UniqueId()+1;
	Micro_tls client(client_id.theBytes);
	Micro_tls server(server_id.theBytes);
	unsigned char serverCertificate[server.getCertificateLength()];
	unsigned char clientCertificate[client.getCertificateLength()];

	//We're not using certificate chains here, so the certificate must be distributed by secure means to the parties involved.
	server.exportCertificate(serverCertificate);
	client.exportCertificate(clientCertificate);
	server.importCertificate(clientCertificate);
	client.importCertificate(serverCertificate);

	uint8_t e_pubkey_client[crypto_kx_PUBLICKEYBYTES];
	uint8_t cookie_client[32];
	uint8_t f_pubkey_server[crypto_kx_PUBLICKEYBYTES];
	uint8_t cookie_server[32];
	int cookielength, e_pubkey_length;

	//Key exchange protocol
	//Step 1 : Client sends ClientHello to server : (client_cookie | e)
	client.generateHello(cookie_client, cookielength, e_pubkey_client, e_pubkey_length);
	printArray("cookie_client: ", cookie_client, cookielength);
	printArray("e: ", e_pubkey_client, e_pubkey_length);

	//Step 2 : Server generates cookie and a public key for key exchange "f" 
	server.generateHello(cookie_server, cookielength, f_pubkey_server, e_pubkey_length);
	printArray("cookie_server: ", cookie_server, cookielength);
	printArray("f: ", f_pubkey_server, e_pubkey_length);
	
	//Step 3 : Server calculates handshake secret and exchange hash and then signs the exchange hash
	server.calcHandshakeSecret(e_pubkey_client, false);
	server.calcExchangeHash(cookie_client, false);
	unsigned char sig[server.getSignatureLength()];
	server.signExchangeHash(sig);
	printArray("s: ", sig, sizeof sig);

	//Step 4 : Server sends cookie, public key for key exchange "f" and signature to client : (server_cookie | f | s)

	//Step 5 : Client calculates handshake secret and exchange hash and then verifies the signature of the exchange hash
	client.calcHandshakeSecret(f_pubkey_server, true);
	client.calcExchangeHash(cookie_server, true);
	Serial.printf("Signature verification: %s\r\n", client.checkSignature(sig) ? "ok" : "fail");

	Serial.println("setup done.");
}

void loop()
{
	// put your main code here, to run repeatedly:
}