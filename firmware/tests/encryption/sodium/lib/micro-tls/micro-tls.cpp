#include "micro-tls.h"
#include "string.h"

//to be removed later on, only for debug
extern void printArray(const char *str, const uint8_t *buf, size_t nbytes);

Micro_tls::Micro_tls(uint32_t id) : _id(id)
{
    if (sodium_init() < 0)
    {
        /* panic! the library couldn't be initialized, it is not safe to use */
        while (true)
            ;
    }
}

Micro_tls::Micro_tls(uint32_t id, uint8_t *pubKey, uint8_t *secretKey) : Micro_tls(id)
{
    memcpy(_public_key_static, pubKey, crypto_box_PUBLICKEYBYTES);
    memcpy(_private_key_static, secretKey, crypto_box_SECRETKEYBYTES);
    _isInitStaticKey = true;
}

Micro_tls::~Micro_tls()
{
    sodium_memzero(_private_key_static, crypto_box_SECRETKEYBYTES);
    sodium_memzero(_private_key_ephemeral, crypto_kx_SECRETKEYBYTES);
    sodium_memzero(_handshakeSecret_K_part1, crypto_kx_SESSIONKEYBYTES);
    sodium_memzero(_handshakeSecret_K_part2, crypto_kx_SESSIONKEYBYTES);
}

void Micro_tls::createStaticKeyPair()
{
    crypto_box_keypair(_public_key_static, _private_key_static);
    _isInitStaticKey = true;
}

size_t Micro_tls::getStaticKeylength()
{
    return sizeof _public_key_static;
}

void Micro_tls::getStaticPublicKey(uint8_t *key)
{
    memcpy(key, _public_key_static, sizeof _public_key_static);
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

    memcpy(cookie, _random, sizeof(_random));
    cookieLength = sizeof(_random);
    memcpy(public_ephemeral_key, _public_key_ephemeral, sizeof(_public_key_ephemeral));
    keyLength = sizeof(_public_key_ephemeral);
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
    printArray("K, part1: ", _handshakeSecret_K_part1, crypto_kx_SESSIONKEYBYTES);
    printArray("K, part2: ", _handshakeSecret_K_part2, crypto_kx_SESSIONKEYBYTES);
    return true;
}

/**
 * https://datatracker.ietf.org/doc/html/rfc4253#section-8
 * The exchange hash H is considered secret data.
 */
bool Micro_tls::calcExchangeHash(uint32_t peerId, uint8_t *peerCookie, uint8_t *serverStaticKey, bool isClient)
{
    unsigned char hash[crypto_generichash_BYTES];
    crypto_generichash_state state;
    crypto_generichash_init(&state, _handshakeSecret_K_part1, sizeof _handshakeSecret_K_part1, sizeof hash);
    crypto_generichash_update(&state, (uint8_t *)(isClient ? &_id : &peerId), 4);
    crypto_generichash_update(&state, (uint8_t *)(isClient ? &peerId : &_id), 4);
    crypto_generichash_update(&state, isClient ? _random : peerCookie, sizeof _random);
    crypto_generichash_update(&state, isClient ? peerCookie : _random, sizeof _random);
    crypto_generichash_update(&state, isClient ? serverStaticKey : _public_key_static, crypto_box_PUBLICKEYBYTES);
    crypto_generichash_update(&state, isClient ? _public_key_ephemeral : _public_key_ephemeral_peer, sizeof _public_key_ephemeral);
    crypto_generichash_update(&state, isClient ? _public_key_ephemeral_peer : _public_key_ephemeral, sizeof _public_key_ephemeral);
    crypto_generichash_update(&state,_handshakeSecret_K_part2, sizeof _handshakeSecret_K_part2);
    crypto_generichash_final(&state, hash, sizeof hash);
    printArray("Exchange hash H: ", hash, sizeof hash);
    return false;
}
