#include "micro-tls.h"
#include "string.h"

//to be removed later on, only for debug
extern void printArray(const char *str, const uint8_t *buf, size_t nbytes);

Micro_tls::Micro_tls()
{
    if (sodium_init() < 0)
    {
        /* panic! the library couldn't be initialized, it is not safe to use */
        while (true)
            ;
    }
    sodium_memzero(_hash_H, sizeof _hash_H);
}

Micro_tls::Micro_tls(uint8_t *id): Micro_tls()
{
    memcpy(_myCertificate.id, id, sizeof _myCertificate.id);
    /** Create a static key pair for the object.
     * This should only be called once in the lifetime of the product.  The result should then be exported and saved in non-volatile memory.
     * This key-pair is used to create a certificate, which will then be distributed among devices in the same group for authentication purposes.
     */
    crypto_sign_keypair(_myCertificate.public_key_signing, _myCertificate.private_key_signing);
}

/** Create the object with the certificate and secret key
 * \param certificate : contains id, pubkey and self-signed signature
 * \param secret key : secret key belonging to pubkey in the certificate
 */
Micro_tls::Micro_tls(uint8_t *certificate, uint8_t *secretKey): Micro_tls()
{
    if (!importCertificate(certificate))
    {
        while (true)
            ;
    }
    memcpy(_myCertificate.private_key_signing, secretKey, sizeof _myCertificate.private_key_signing);
}

//Clean up all sensitive data
Micro_tls::~Micro_tls()
{
    sodium_memzero(_myCertificate.private_key_signing, sizeof _myCertificate.private_key_signing);
    sodium_memzero(_private_key_ephemeral, sizeof _private_key_ephemeral);
    sodium_memzero(_handshakeSecret_K_part1, sizeof _handshakeSecret_K_part1);
    sodium_memzero(_handshakeSecret_K_part2, sizeof _handshakeSecret_K_part2);
}

size_t Micro_tls::getCertificateLength()
{
    return sizeof _myCertificate.id + sizeof _myCertificate.public_key_signing + getSignatureLength();
}

/** Export certificate data as self-signed certificate.
 * Cryptographically self-signed certificates don't mean much.  Everyone could simply replace the pubkey and sign the certificate again
 * with the private key that matches the replacing pubkey.  Self-signing only assures that the certificate data is still valid.
 */
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

// Import and check self-signed certificate
bool Micro_tls::importCertificate(uint8_t *certificate)
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
    memcpy(_peerCertificate.id, unsigned_message, sizeof _myCertificate.id);
    memcpy(_peerCertificate.public_key_signing, cert_pub_key, sizeof cert_pub_key);
    return true;
}

/** Generate the first message in the key exchange protocol.
 * It's the client who sends the output of this command to the server.
 * \param cookie will contain a random value upon return, to prevent replay-attacks.
 * \param cookieLength size of the cookie
 * \param public_ephemeral_key public key for key exchange purposes.  Short lifetime.
 * \param keyLength size of the public ephemeral key
 */
void Micro_tls::generateHello(uint8_t *cookie, int &cookieLength, uint8_t *public_ephemeral_key, int &keyLength)
{
    //Generate random
    randombytes_buf(_random, sizeof _random);
    //Generate ephemeral key pair
    crypto_kx_keypair(_public_key_ephemeral, _private_key_ephemeral);

    memcpy(cookie, _random, sizeof _random);
    cookieLength = sizeof _random;
    memcpy(public_ephemeral_key, _public_key_ephemeral, sizeof _public_key_ephemeral);
    keyLength = sizeof _public_key_ephemeral;
}

/** Calculate the handshake secret "K"
 * The ephemeral keypair is used here to establish a common secret between client and server.
 * The handshake secret(!) is an intermediary value that will be used later on to derive the traffic keydata from.
 * The handshake secret will be calculated by the client and the server independently.  They should arrive at the same value.
 * https://datatracker.ietf.org/doc/html/rfc4253#section-8 names this value "K".
 * Instead of using Diffie-Hellman, libsodium uses a hash function by default. 
 * \param public_key_ephemeral_peer epthemeral pubkey of the other party
 * \param isClient client=true, server=false.  This is used to reconstruct the handshake secret from libsodium's "traffic keys".
 * \return true when public key of the remote party is valid
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

/** Calculate the exchange hash H.
 * H is also an intermediate value used to derive traffic keydata from.  Both client and server will generate this hash.  They should independently 
 * arrive at the same value.
 * https://datatracker.ietf.org/doc/html/rfc4253#section-8
 * The exchange hash H is considered secret data.
 * \param peerId id of the other party
 * \param peerCookie cookie of the other party
 * \param serverCertificate client passes in certificate of the server.  The server fills in a nullptr here.
 * \param isClient true for client, false for server.
 */
void Micro_tls::calcExchangeHash(uint8_t *peerCookie, bool isClient)
{
    crypto_generichash_state state;
    crypto_generichash_init(&state, _handshakeSecret_K_part1, sizeof _handshakeSecret_K_part1, sizeof _hash_H);
    crypto_generichash_update(&state, isClient ? _myCertificate.id : _peerCertificate.id, sizeof _myCertificate.id);
    crypto_generichash_update(&state, isClient ? _peerCertificate.id : _myCertificate.id, sizeof _myCertificate.id);
    crypto_generichash_update(&state, isClient ? _random : peerCookie, sizeof _random);
    crypto_generichash_update(&state, isClient ? peerCookie : _random, sizeof _random);
    crypto_generichash_update(&state, isClient ? _peerCertificate.public_key_signing : _myCertificate.public_key_signing, sizeof _myCertificate.public_key_signing);
    crypto_generichash_update(&state, isClient ? _public_key_ephemeral : _public_key_ephemeral_peer, sizeof _public_key_ephemeral);
    crypto_generichash_update(&state, isClient ? _public_key_ephemeral_peer : _public_key_ephemeral, sizeof _public_key_ephemeral);
    crypto_generichash_update(&state, _handshakeSecret_K_part2, sizeof _handshakeSecret_K_part2);
    crypto_generichash_final(&state, _hash_H, sizeof _hash_H);
    printArray("Exchange hash H: ", _hash_H, sizeof _hash_H);
}

size_t Micro_tls::getSignatureLength()
{
    return crypto_sign_BYTES;
}

/** Sign the exchange hash with the private host key
 * According to the SSH protocol key exchange (https://datatracker.ietf.org/doc/html/rfc4253#section-8)
 */
bool Micro_tls::signExchangeHash(uint8_t *public_key_ephemeral_peer, uint8_t *peerCookie, bool isClient, uint8_t *signature)
{
    if (!calcHandshakeSecret(public_key_ephemeral_peer, isClient))
    {
        return false;
    }
    calcExchangeHash(peerCookie, isClient);
    return signExchangeHash(signature);
}

/** Sign the exchange hash
 * The hash should have been generated beforehand.
 */
bool Micro_tls::signExchangeHash(uint8_t *signature)
{
    if(sodium_is_zero(_hash_H, sizeof _hash_H)==1)
    {
        return false;
    }
    crypto_sign_detached(signature, nullptr, _hash_H, sizeof _hash_H, _myCertificate.private_key_signing);
    printArray("s: ", signature, getSignatureLength());
    return true;
}

/** Verify the signature using the public key provided by the certificate.
 * \param certificate The certificate of the remote party.  This certificate must be pre-installed on this device.
 * \param signature The signature that must be verified.
 */
bool Micro_tls::finishKeyExchange(uint8_t *public_key_ephemeral_peer, uint8_t *peerCookie, bool isClient, uint8_t *signature)
{
    if (!calcHandshakeSecret(public_key_ephemeral_peer, isClient))
    {
        return false;
    }
    calcExchangeHash(peerCookie, isClient);
    return finishKeyExchange(signature, isClient);
}

/** Check the signature and generate the traffic keys.
 * The needed hash should have been generated beforehand.
 */
bool Micro_tls::finishKeyExchange(uint8_t *signature, bool isClient)
{
    if(crypto_sign_verify_detached(signature, _hash_H, sizeof _hash_H, _peerCertificate.public_key_signing) != 0)
    {
        return false;
    }
    deriveKey('A', isClient ? traffic_iv_tx : traffic_iv_rx, sizeof traffic_iv_tx);     //Initial IV client to server
    deriveKey('B', isClient ? traffic_iv_rx : traffic_iv_tx, sizeof traffic_iv_tx);     //Initial IV server to client
    deriveKey('C', isClient ? traffic_key_tx : traffic_key_rx, sizeof traffic_key_tx);  //Encryption key client to server
    deriveKey('D', isClient ? traffic_key_rx : traffic_key_tx, sizeof traffic_key_tx);  //Encryption key server to client
    return true;
}

/** Derive the traffic key data
 * https://datatracker.ietf.org/doc/html/rfc4253#section-7.2
 */
void Micro_tls::deriveKey(char c, uint8_t *key, size_t keyLength)
{
    crypto_generichash_state state;
    crypto_generichash_init(&state, nullptr, 0, keyLength);
    crypto_generichash_update(&state, _handshakeSecret_K_part1, sizeof _handshakeSecret_K_part1);
    crypto_generichash_update(&state, _handshakeSecret_K_part2, sizeof _handshakeSecret_K_part2);
    crypto_generichash_update(&state, _hash_H, sizeof _hash_H);
    crypto_generichash_update(&state, (uint8_t*)&c, 1);
    crypto_generichash_final(&state, key, keyLength);
    char buf[10];
    sprintf(buf, "%c key: ", c);
    printArray(buf, key, keyLength);
}