#pragma once

#include <sodium.h>

class Micro_tls
{
public:
    typedef struct
    {
        uint8_t public_key_signing[crypto_sign_PUBLICKEYBYTES];
        uint8_t private_key_signing[crypto_sign_SECRETKEYBYTES]; //!< This must be kept secret.
        uint8_t id[4];
    } certificateData;
    Micro_tls(uint8_t *id);
    Micro_tls(uint8_t *certificate, uint8_t *secretKey);
    void exportCertificate(uint8_t *certificate);
    bool importCertificate(uint8_t *certificate);
    size_t getSignatureLength();
    size_t getCertificateLength();
    void generateHello(uint8_t *cookie, int &cookieLength, uint8_t *public_ephemeral_key, int &keyLength);
    bool signExchangeHash(uint8_t *public_key_ephemeral_peer, uint8_t *peerCookie, bool isClient, uint8_t *signature);
    bool finishKeyExchange(uint8_t *public_key_ephemeral_peer, uint8_t *peerCookie, bool isClient, uint8_t *signature);
    bool signExchangeHash(uint8_t *signature);
    bool finishKeyExchange(uint8_t *signature, bool isClient);
    bool encrypt(uint8_t* plaintext, size_t plaintextLength, uint8_t* ciphertext, size_t& ciphertextLength);
    bool decrypt(uint8_t* ciphertext, size_t ciphertextLength, uint8_t* plaintext, size_t& plaintextLength);
    ~Micro_tls();

private:
    Micro_tls();
    bool calcHandshakeSecret(uint8_t *public_key_ephemeral_peer, bool isClient);
    void calcExchangeHash(uint8_t *peerCookie, bool isClient);
    void deriveKey(char c, uint8_t *key, size_t keyLength);
    void generateNonce(uint8_t* nonce, size_t nonceLength, uint8_t* initial_iv, uint8_t* ctr, size_t ctrLength);
    uint8_t _public_key_ephemeral[crypto_kx_PUBLICKEYBYTES];
    uint8_t _public_key_ephemeral_peer[crypto_kx_PUBLICKEYBYTES];
    uint8_t _private_key_ephemeral[crypto_kx_SECRETKEYBYTES];
    uint8_t _random[32];
    uint8_t _handshakeSecret_K_part1[crypto_kx_SESSIONKEYBYTES];
    uint8_t _handshakeSecret_K_part2[crypto_kx_SESSIONKEYBYTES];
    uint8_t _hash_H[crypto_generichash_BYTES];
    certificateData _myCertificate;
    certificateData _peerCertificate;
    uint8_t traffic_iv_tx[crypto_secretbox_NONCEBYTES]={0};
    uint8_t traffic_iv_rx[crypto_secretbox_NONCEBYTES]={0};
    uint8_t traffic_key_tx[crypto_secretbox_KEYBYTES]={0};
    uint8_t traffic_key_rx[crypto_secretbox_KEYBYTES]={0};
    uint8_t traffic_ctr_tx[4]={0};  //!< Increased by one when a packet is sent
};
