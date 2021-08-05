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
    bool checkSignature(uint8_t *public_key_ephemeral_peer, uint8_t *peerCookie, bool isClient, uint8_t *signature);
    void signExchangeHash(uint8_t *signature);
    bool checkSignature(uint8_t *signature);
    ~Micro_tls();

private:
    bool calcHandshakeSecret(uint8_t *public_key_ephemeral_peer, bool isClient);
    void calcExchangeHash(uint8_t *peerCookie, bool isClient);

    uint8_t _public_key_ephemeral[crypto_kx_PUBLICKEYBYTES];
    uint8_t _public_key_ephemeral_peer[crypto_kx_PUBLICKEYBYTES];
    uint8_t _private_key_ephemeral[crypto_kx_SECRETKEYBYTES];
    uint8_t _random[32];
    uint8_t _handshakeSecret_K_part1[crypto_kx_SESSIONKEYBYTES];
    uint8_t _handshakeSecret_K_part2[crypto_kx_SESSIONKEYBYTES];
    uint8_t _hash_H[crypto_generichash_BYTES];
    certificateData _myCertificate;
    certificateData _peerCertificate;
};
