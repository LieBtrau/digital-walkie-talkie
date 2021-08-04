#include "micro-tls.h"
#include "string.h"

//to be removed later on, only for debug
extern void printArray(const char *str, const uint8_t *buf, size_t nbytes);

Micro_tls::Micro_tls(uint8_t *id)
{
    if (sodium_init() < 0)
    {
        /* panic! the library couldn't be initialized, it is not safe to use */
        while (true)
            ;
    }
    memcpy(_myCertificate.id, id, sizeof _myCertificate.id);
}

Micro_tls::Micro_tls(uint8_t *certificate, uint8_t *secretKey)
{
    if (sodium_init() < 0)
    {
        /* panic! the library couldn't be initialized, it is not safe to use */
        while (true)
            ;
    }
    if (!importCertificate(certificate, &_myCertificate))
    {
        while (true)
            ;
    }
    memcpy(_myCertificate.private_key_signing, secretKey, sizeof _myCertificate.private_key_signing);
    _isInitStaticKey = true;
}

Micro_tls::~Micro_tls()
{
    sodium_memzero(_myCertificate.private_key_signing, sizeof _myCertificate.private_key_signing);
    sodium_memzero(_private_key_ephemeral, sizeof _private_key_ephemeral);
    sodium_memzero(_handshakeSecret_K_part1, sizeof _handshakeSecret_K_part1);
    sodium_memzero(_handshakeSecret_K_part2, sizeof _handshakeSecret_K_part2);
}

void Micro_tls::createSigningKeyPair()
{
    crypto_sign_keypair(_myCertificate.public_key_signing, _myCertificate.private_key_signing);
    _isInitStaticKey = true;
}

bool Micro_tls::generateHello(uint8_t *cookie, int &cookieLength, uint8_t *public_ephemeral_key, int &keyLength)
{
    if (!_isInitStaticKey)
    {
        return false;
    }
    //Generate random
    randombytes_buf(_random, sizeof _random);
    //Generate ephemeral key pair
    crypto_kx_keypair(_public_key_ephemeral, _private_key_ephemeral);

    memcpy(cookie, _random, sizeof _random);
    cookieLength = sizeof _random;
    memcpy(public_ephemeral_key, _public_key_ephemeral, sizeof _public_key_ephemeral);
    keyLength = sizeof _public_key_ephemeral;
    return true;
}

/**
 * https://datatracker.ietf.org/doc/html/rfc4253#section-8
 * Instead of using Diffie-Hellman, libsodium uses a hash function by default. 
 */
bool Micro_tls::calcHandshakeSecret(uint8_t *public_key_ephemeral_peer, bool isClient)
{
    memcpy(_public_key_ephemeral_peer, public_key_ephemeral_peer, sizeof _public_key_ephemeral_peer);
    if (isClient)
    {
        if (crypto_kx_client_session_keys(_handshakeSecret_K_part1, _handshakeSecret_K_part2, _public_key_ephemeral, _private_key_ephemeral, public_key_ephemeral_peer) != 0)
        {
            /* Suspicious server public key, bail out */
            return false;
        }
    }
    else
    {
        if (crypto_kx_server_session_keys(_handshakeSecret_K_part2, _handshakeSecret_K_part1, _public_key_ephemeral, _private_key_ephemeral, public_key_ephemeral_peer) != 0)
        {
            /* Suspicious client public key, bail out */
            return false;
        }
    }
    printArray("K, part1: ", _handshakeSecret_K_part1, sizeof _handshakeSecret_K_part1);
    printArray("K, part2: ", _handshakeSecret_K_part2, sizeof _handshakeSecret_K_part2);
    return true;
}

/**
 * https://datatracker.ietf.org/doc/html/rfc4253#section-8
 * The exchange hash H is considered secret data.
 */
bool Micro_tls::calcExchangeHash(uint8_t *peerId, uint8_t *peerCookie, uint8_t *peerCertificate, bool isClient)
{
    certificateData peerCert;
    if (isClient && !importCertificate(peerCertificate, &peerCert))
    {
        return false;
    }
    crypto_generichash_state state;
    crypto_generichash_init(&state, _handshakeSecret_K_part1, sizeof _handshakeSecret_K_part1, sizeof _hash_H);
    crypto_generichash_update(&state, isClient ? _myCertificate.id : peerId, sizeof _myCertificate.id);
    crypto_generichash_update(&state, isClient ? peerId : _myCertificate.id, sizeof _myCertificate.id);
    crypto_generichash_update(&state, isClient ? _random : peerCookie, sizeof _random);
    crypto_generichash_update(&state, isClient ? peerCookie : _random, sizeof _random);
    crypto_generichash_update(&state, isClient ? peerCert.public_key_signing : _myCertificate.public_key_signing, sizeof _myCertificate.public_key_signing);
    crypto_generichash_update(&state, isClient ? _public_key_ephemeral : _public_key_ephemeral_peer, sizeof _public_key_ephemeral);
    crypto_generichash_update(&state, isClient ? _public_key_ephemeral_peer : _public_key_ephemeral, sizeof _public_key_ephemeral);
    crypto_generichash_update(&state, _handshakeSecret_K_part2, sizeof _handshakeSecret_K_part2);
    crypto_generichash_final(&state, _hash_H, sizeof _hash_H);
    printArray("Exchange hash H: ", _hash_H, sizeof _hash_H);
    return true;
}

size_t Micro_tls::getSignatureLength()
{
    return crypto_sign_BYTES;
}

/**
 * Sign the exchange hash with the private host key
 */
void Micro_tls::signExchangeHash(uint8_t *signature)
{
    crypto_sign_detached(signature, NULL, _hash_H, sizeof _hash_H, _myCertificate.private_key_signing);
}

size_t Micro_tls::getCertificateLength()
{
    return sizeof _myCertificate.id + sizeof _myCertificate.public_key_signing + getSignatureLength();
}

// Export certificate data as self-signed certificate.
void Micro_tls::exportCertificate(uint8_t *certificate)
{
    uint8_t message[sizeof _myCertificate.id + sizeof _myCertificate.public_key_signing];
    printArray("Export: id: ", _myCertificate.id, sizeof _myCertificate.id);
    printArray("Export: pubkey: ", _myCertificate.public_key_signing, sizeof _myCertificate.public_key_signing);
    memcpy(message, _myCertificate.id, sizeof _myCertificate.id);
    memcpy(message + sizeof _myCertificate.id, _myCertificate.public_key_signing, sizeof _myCertificate.public_key_signing);
    crypto_sign(certificate, nullptr, message, sizeof message, _myCertificate.private_key_signing);
    printArray("Certificate: ", certificate, getCertificateLength());
}

bool Micro_tls::importCertificate(uint8_t *certificate, certificateData *imported)
{
    uint8_t cert_pub_key[sizeof _myCertificate.public_key_signing];
    unsigned char unsigned_message[sizeof _myCertificate.id + sizeof _myCertificate.public_key_signing];
    unsigned long long unsigned_message_len;
    printArray("Import: id: ", certificate + getSignatureLength(), sizeof _myCertificate.id);
    printArray("Import: pubkey: ", certificate + getSignatureLength() + sizeof _myCertificate.id, sizeof _myCertificate.public_key_signing);
    //Extract pubkey from certificate (signature | id | pubkey)
    memcpy(cert_pub_key, certificate + getSignatureLength() + sizeof _myCertificate.id, sizeof _myCertificate.public_key_signing);
    //Check imported certificate
    if (crypto_sign_open(unsigned_message, &unsigned_message_len, certificate, getCertificateLength(), cert_pub_key) != 0)
    {
        /* Incorrect signature! */
        return false;
    }
    memcpy(imported->id, unsigned_message, sizeof _myCertificate.id);
    memcpy(imported->public_key_signing, cert_pub_key, sizeof cert_pub_key);
    return true;
}

// /** Extract the public key from the certificate and use it to verify the signature.
//  * \param certificate The certificate of the remote party.  This certificate must be pre-installed on this device.
//  * \param signature The signature that must be verified.
//  */
// bool Micro_tls::checkSignature(uint8_t *certificate, uint8_t *signature)
// {
//     unsigned char unsigned_message[sizeof _myCertificate.id];
//     unsigned long long unsigned_message_len;
//     if (crypto_sign_open(unsigned_message, &unsigned_message_len, certificate, getCertificateLength(), publicKey) != 0)
//     {
//         /* Incorrect signature! */
//         return false;
//     }
//     if (sodium_compare(unsigned_message, peerId, sizeof _myCertificate.id) != 0)
//     {
//         //This certificate belongs to another ID.
//         return false;
//     }
//     return true;
// }
