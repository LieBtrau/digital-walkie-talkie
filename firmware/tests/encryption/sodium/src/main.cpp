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

uint32_t getEsp32UniqueId()
{
	uint32_t chipId = 0;
	for (int i = 0; i < 17; i = i + 8)
	{
		chipId |= ((ESP.getEfuseMac() >> (40 - i)) & 0xff) << i;
	}
	return chipId;
}

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
	server_id.theDWord = getEsp32UniqueId() + 1;
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
	unsigned char sig[server.getSignatureLength()];
	if (!server.signExchangeHash(e_pubkey_client, cookie_client, false, sig))
	{
		return;
	}

	//Step 4 : Server sends cookie, public key for key exchange "f" and signature to client : (server_cookie | f | s)

	//Step 5 : Client calculates handshake secret and exchange hash and then verifies the signature of the exchange hash
	bool sigOk = client.finishKeyExchange(f_pubkey_server, cookie_server, true, sig);
	Serial.printf("Signature verification: %s\r\n", sigOk ? "ok" : "fail");
	if (!sigOk)
	{
		return;
	}
	//At this point, the client knows that the server is authentic.

	//Step 6 : To provide mutual authentication, the client will also generate a signature, signed by its own private key.
	client.signExchangeHash(sig);

	//Step 7 : Clients sends its signature to the server : (s)

	//Step 8 : The server checks the client's signature
	sigOk = server.finishKeyExchange(sig, false);
	Serial.printf("Signature verification: %s\r\n", sigOk ? "ok" : "fail");
	if (!sigOk)
	{
		return;
	}
	Serial.println("Mutual authentication established");

	//Symmetric keys have been securely exchanged.  Time to use them...
	uint8_t plaintext[] = {1, 2, 3, 4, 5};
	uint8_t decrypted[20];
	uint8_t ciphertext[100];
	size_t ciphertextlength, decryptedLength;
	//Ciphertext should be different each run, although we're encrypting the same data.
	for (int i = 0; i < 3; i++)
	{
		if (!client.encrypt(plaintext, sizeof plaintext, ciphertext, ciphertextlength))
		{
			return;
		}
		printArray("CipherText: ", ciphertext, ciphertextlength);
		Serial.printf("Ciphertext length: %d\r\n", ciphertextlength);
		if (!server.decrypt(ciphertext, ciphertextlength, decrypted, decryptedLength))
		{
			return;
		}
		printArray("Decrypted: ", decrypted, decryptedLength);
	}
}

void loop()
{
	// put your main code here, to run repeatedly:
}