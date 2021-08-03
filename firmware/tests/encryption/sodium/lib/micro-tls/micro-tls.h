#pragma once

#include <sodium.h>

class Micro_tls
{
private:
    uint8_t _public_key_static[crypto_box_PUBLICKEYBYTES];
    uint8_t _private_key_static[crypto_box_SECRETKEYBYTES];
    uint8_t _public_key_ephemeral[crypto_kx_PUBLICKEYBYTES];
    uint8_t _public_key_ephemeral_peer[crypto_kx_PUBLICKEYBYTES];
    uint8_t _private_key_ephemeral[crypto_kx_SECRETKEYBYTES];
    uint8_t _random[32];
    uint8_t _handshakeSecret_K_part1[crypto_kx_SESSIONKEYBYTES];
    uint8_t _handshakeSecret_K_part2[crypto_kx_SESSIONKEYBYTES];
    bool _isInitStaticKey = false;
    uint32_t _id;

public:
    Micro_tls(uint32_t id);
    Micro_tls(uint32_t id, uint8_t *pubKey, uint8_t *secretKey);
    void createStaticKeyPair();
    size_t getStaticKeylength();
    void getStaticPublicKey(uint8_t* key);
    bool generateHello(uint8_t *cookie, int &cookieLength, uint8_t* public_ephemeral_key, int& keyLength);
    bool calcHandshakeSecret(uint8_t *public_key_ephemeral_peer, bool isClient);
    bool calcExchangeHash(uint32_t peerId, uint8_t *peerCookie, uint8_t* peerStaticKey, bool isClient); //todo: this shouldn't be a public function
    ~Micro_tls();
};
