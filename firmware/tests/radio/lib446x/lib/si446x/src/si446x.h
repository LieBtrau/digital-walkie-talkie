//https://github.com/UBNanosatLab/lib446x

#ifndef SI446X_H
#define SI446X_H

#include <stdbool.h>
#include <stdint.h>

#define ESUCCESS 0 /**< @brief command succeeded */
#define ETIMEOUT 1 /**< @brief timeout waiting for CTS*/
#define EWRONGPART 2 /**< @brief unsupported part number*/
#define EINVAL 3 /**< @brief invalid parameter*/
#define EINVALSTATE 4 /**< @brief invalid internal state*/
#define ETOOLONG 5 /**< @brief packet too long*/
#define ECHKSUM 6 /**< @brief invalid check sum*/
#define EBUSY 7 /**< @brief pending operation*/
#define ESILICON 8 /**< @brief Si446x silicon bug, try again*/
#define ERESETSI 9 /**< @brief Si446x silicon bug, reset RFIC and try again*/

#define ENOTIMPL 127  /**< @brief not yet implemented functionality*/

#define CRC_SEED_0                                          0x00
#define CRC_SEED_1                                          0x80
#define CRC_NO_CRC                                          0x00
#define CRC_ITU_T_8                                         0x01
//...
#define CRC_IBM_16                                          0x04
#define CRC_CCIT_16                                         0x05

#define OPT_XTAL                                            0x00
#define OPT_TCXO                                            0x01

#define DIRECT_MODE_SYNC                                    0x00
#define DIRECT_MODE_ASYNC                                   0x80

#define DIRECT_MODE_GPIO0                                   0x00
#define DIRECT_MODE_GPIO1                                   0x20
#define DIRECT_MODE_GPIO2                                   0x40
#define DIRECT_MODE_GPIO3                                   0x60

#define MOD_SRC_PKT                                         0x00
#define MOD_SRC_PIN                                         0x08
#define MOD_SRC_RAND                                        0x10

#define MOD_TYPE_CW                                         0x00
#define MOD_TYPE_OOK                                        0x01
#define MOD_TYPE_2FSK                                       0x02
#define MOD_TYPE_2GFSK                                      0x03
#define MOD_TYPE_4FSK                                       0x04
#define MOD_TYPE_4GFSK                                      0x05

#define GPIO_DONOTHING                                      0x00
#define GPIO_TRISTATE                                       0x01
// ...
#define GPIO_CTS                                            0x08
#define GPIO_INV_CTS                                        0x09
#define GPIO_CMD_OVERLAP                                    0x0A
// ...
#define GPIO_EN_PA                                          0x0F
#define GPIO_TX_DATA_CLK                                    0x10
#define GPIO_RX_DATA_CLK                                    0x11
#define GPIO_EN_LNA                                         0x12
#define GPIO_TX_DATA                                        0x13
#define GPIO_RX_DATA                                        0x14
#define GPIO_RX_RAW_DATA                                    0x15
// ...
#define GPIO_SYNC_WORD_DETECT                               0x1A
// ...
#define GPIO_TX_STATE                                       0x20
#define GPIO_RX_STATE                                       0x21
#define GPIO_RX_FIFO_FULL                                   0x22
#define GPIO_TX_FIFO_EMPTY                                  0x23
// ...



#define MAX_PACKET_SIZE 255 /**< @brief maximum packet size in bytes*/
#define FIFO_SIZE 64
#define FIFO_INIT_FILL 56
#define RX_FIFO_THRESH 16
#define TX_FIFO_THRESH 48

/**
 * @brief The internal state of the library
 */
enum si446x_state {
    IDLE,
    LISTEN,
    RECEIVING,
    TRANSMITTING
};


/**
 * @brief Represents a buffer with a length and position
 */
struct buffer {
    uint8_t *data;
    int len;
};

/**
 *  @brief Represents a unique Si446x device
 */
struct si446x_device {
    uint32_t xo_freq;
    uint16_t part;
    int nsel_pin;
    int sdn_pin;
    int int_pin;
    void (*pkt_tx_handler)(struct si446x_device *dev, int err);
    void (*pkt_rx_handler)(struct si446x_device *dev, int err, int len,
                          uint8_t *data);
    volatile struct buffer tx_buf;
    volatile struct buffer rx_buf;
    volatile uint8_t rx_pkt_len;
    volatile uint8_t tx_pkt_index;
    volatile uint8_t rx_pkt_index;
    volatile bool has_rx_len;
    uint8_t config;
    enum si446x_state state;
    int err; // Used in blocking send / recv
    volatile bool rx_timeout;
};

/**
 *  @brief Info about an Si446x part
 */
struct si446x_part_info {
    uint8_t chip_rev;   /**< @brief Chip revision */
    uint16_t part;      /**< @brief Part number (e.g. 0x4463) */
    uint8_t part_build; /**< @brief Part build */
    uint16_t id;        /**< @brief ID number */
    uint8_t customer;   /**< @brief Customer */
    uint8_t rom_id;     /**< @brief ROM ID */
};

/**
 * Populate an si446x_device struct
 * @param dev The structure to populate
 * @param nsel_pin The NSEL (chip select) pin number
 * @param sdn_pin The SDN (shutdown) pin number
 * @param int_pin The INT (interrupt) pin number
 * @param xo_freq The frequency of the crystal attached to the Si446x
 * @param xo_freq Configuration parameters: xtal type
 * @return negative error code or 0 for success
 */
int si446x_create(struct si446x_device *dev, int nsel_pin, int sdn_pin,
                  int int_pin, uint32_t xo_freq, uint8_t config);

/**
 * Initalize an Si446x device
 * @param device si446x device to initalize
 * @return negative error code or 0 for success
 */
int si446x_init(struct si446x_device *device);

/**
 * Reset and reinitialize the Si446x
 * @return negative error code or 0 for success
 */
int si446x_reinit(struct si446x_device *dev);

/**
 * Get part info
 * @param device the si446x device
 * @param info part_info structure to populate
 * @return negative error code or 0 for success
 */
int si446x_get_part_info(struct si446x_device *device,
                         struct si446x_part_info *info);

/**
 * Set the transmit / receive frequency
 * @param device the si446x device
 * @param freq the desired fc value (See SiLabs datasheet, not Hz!)
 * @param freq the desired band setting value
 *             (See SiLabs datasheet, depends on 4463/4464)
 * @return negative error code or 0 for success
 */
int si446x_set_frequency(struct si446x_device *dev, uint32_t fc, uint8_t band);

/**
 * Configure the CRC polynomial
 * @param device the si446x device
 * @param cfg the CRC polynomial and seed value
 * @return negative error code or 0 for success
 */
int si446x_config_crc(struct si446x_device *dev, uint8_t cfg);

/**
 * Set Si446x transmit power
 * @param device the si446x device
 * @param pwr power setting (see graph in datasheet)
 * @return negative error code or 0 for success
 */
int si446x_set_tx_pwr(struct si446x_device *dev, uint8_t pwr);

/**
 * Set Si446x modulation type (e.g. OOK, GFSK, pseudorandom noise)
 * @param device the si446x device
 * @param mod_type modulation type
 * @return negative error code or 0 for success
 */
int si446x_set_mod_type(struct si446x_device *dev, uint8_t mod_type);

/**
 * Set GPIO pin configurations
 * @param device the si446x device
 * @param gpio0 config for GPIO 0
 * @param gpio1 config for GPIO 1
 * @param gpio2 config for GPIO 2
 * @param gpio3 config for GPIO 3
 * @return negative error code or 0 for success
 */
int si446x_cfg_gpio(struct si446x_device *dev, uint8_t gpio0, uint8_t gpio1,
                    uint8_t gpio2, uint8_t gpio3);

/**
 * Transmit the provided data
 * Note: If this returns -ERESETSI, the si446x is stuck transmitting and 
 * should be *immediately* reset
 * @param device the si446x device
 * @param len the length of the data in bytes
 * @param data pointer to the data
 * @return negative error code or 0 for success
 */
int si446x_send(struct si446x_device *dev, int len, uint8_t *data);

/**
 * Receive data into the provided buffer
 * @param device the si446x device
 * @param len pointer to length of the data in bytes, updated with actual rx len
 * @param data pointer to the data
 * @return negative error code or 0 for success
 */
int si446x_recv(struct si446x_device *dev, int *len, uint8_t *data);

/**
 * Transmit data asynchronously from the provided buffer
 * Note: If this returns -ERESETSI, the si446x is stuck transmitting and 
 * should be *immediately* reset
 * @param device the si446x device
 * @param len pointer to length of the buffer in bytes
 * @param data pointer to the buffer
 * @param handler function to call upon packet completion
 * @return negative error code or 0 for success
 */
int si446x_send_async(struct si446x_device *dev, int len, uint8_t *buf,
                      void (*handler)(struct si446x_device *dev, int err));

/**
 * Receive data asynchronously into the provided buffer
 * @param device the si446x device
 * @param len pointer to length of the buffer in bytes
 * @param data pointer to the buffer
 * @param handler function to call upon packet reception
 * @return negative error code or 0 for success
 */
int si446x_recv_async(struct si446x_device *dev, int len, uint8_t *buf,
                      void (*handler)(struct si446x_device *dev, int err,
                                      int len, uint8_t *data));

/**
 * Update internal state in response to an IRQ
 * @param device the si446x device
 * @return negative error code or 0 for success
 */
int si446x_update(struct si446x_device *dev);

/**
 * Ready the given data for transmission
 * @param device the si446x device
 * @param len the length of the data in bytes
 * @param data pointer to the data
 * @param handler function to call upon packet completion
 * @return negative error code or 0 for success
 */
int si446x_setup_tx(struct si446x_device *dev, int len, uint8_t *data,
                    void (*handler)(struct si446x_device *dev, int err));

/**
 * Transmit the previously setup packet
 * Note: If this returns -ERESETSI, the si446x is stuck transmitting and 
 * should be *immediately* reset
 * @return negative error code or 0 for success
 */
int si446x_fire_tx(struct si446x_device *dev);

/**
 * Indicate that packet RX has timed out
 * @return negative error code or 0 for success
 */
int si446x_rx_timeout(struct si446x_device *dev);

/**
 * Sets if the packet handler should check the RX'd CRC
 * @param device the si446x device
 * @param check_crc should check crc
 * @return negative error code or 0 for success
 */
int si446x_check_crc(struct si446x_device *dev, bool check_crc);

/**
 * Sets if the packet handler should whiten data (for both TX and RX)
 * @param device the si446x device
 * @param whiten should data whitening be enabled
 * @return negative error code or 0 for success
 */
int si446x_data_whitening(struct si446x_device *dev, bool whiten);

/**
 * Ready the given data for transmission
 * @param device the si446x device
 * @param deviation_cfg deviation as a Si446x configuration value (not in Hz!)
 * @return negative error code or 0 for success
 */
int si446x_set_deviation(struct si446x_device *dev, uint32_t deviation_cfg);

/**
 * Send a pre-baked blob of data output from WDS
 * @param device the si446x device
 * @param data_len length of the data
 * @param data the data
 * @return negative error code or 0 for success
 */
int si446x_send_cfg_data_wait(struct si446x_device *dev, int data_len,
                              const uint8_t *data);

/**
 * Abort transmitting (or receiving) and idle
 * Note: Does NOT enter RX mode
 * @return negative error code or 0 for success
 */
int si446x_idle(struct si446x_device *dev);


/**
 * Set a manual frequency offset
 * @param device the si446x device
 * @param offset_counts offset as a Si446x configuration value (not in Hz!)
 * @return negative error code or 0 for success
 */
int si446x_set_freq_offset(struct si446x_device *dev, uint16_t offset_counts);

/**
 * Get radio receive signal strength indicator
 * RSSI is represented as signal power = RSSI * 0.5 dBm/LSB - 134 dBm
 * @param device the si446x device
 * @param rssi the current rssi
 * @param rssi_latch the rssi during the last received packet
 * @return negative error code or 0 for success
 */
int si446x_read_rssi(struct si446x_device *dev, uint8_t *rssi, uint8_t *rssi_latch);

#endif
