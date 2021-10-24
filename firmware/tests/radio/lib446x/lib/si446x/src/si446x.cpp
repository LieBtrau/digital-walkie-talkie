//https://github.com/UBNanosatLab/lib446x
#include "Arduino.h"
#include "SPI.h"
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include "si446x.h"
#include "si446x_cmd.h"
#include "error.h"

#include "radio_config.h"

#define MAX_CTS_RETRY 500

static int send_command(struct si446x_device *dev, uint8_t cmd, int data_len,
                        const uint8_t *data)
{
    digitalWrite(dev->nsel_pin,  LOW);
    SPI.transfer(cmd);
    SPI.writeBytes(data, data_len);
    digitalWrite(dev->nsel_pin,  HIGH);

    return 0;
}

static int send_buffer(struct si446x_device *dev, int data_len, const uint8_t *data)
{
    digitalWrite(dev->nsel_pin,  LOW);
    SPI.writeBytes(data, data_len);
    digitalWrite(dev->nsel_pin,  HIGH);

    return 0;
}

static int wait_cts(struct si446x_device *dev)
{
    uint8_t resp = 0x00;
    uint16_t attempts = 0;

    while (attempts < MAX_CTS_RETRY) {
        digitalWrite(dev->nsel_pin,  LOW);
        SPI.transfer(CMD_READ_CMD_BUF);
        resp = SPI.transfer(0xFF);
        digitalWrite(dev->nsel_pin,  HIGH);

        if (resp == 0xFF) {
            return 0;
        }

        attempts++;

        delayMicroseconds(25);
    }

    return -ETIMEOUT;

}

static uint8_t poll_cts(struct si446x_device *dev)
{
    uint8_t resp = 0x00;

    digitalWrite(dev->nsel_pin,  LOW);
    SPI.transfer(CMD_READ_CMD_BUF);
    resp = SPI.transfer(0xFF);
    digitalWrite(dev->nsel_pin,  HIGH);

    return resp;
}

int si446x_send_cfg_data_wait(struct si446x_device *dev, int data_len,
                              const uint8_t *data)
{
    int err;
    err = send_buffer(dev, data_len, data);

    if (err) {
        return err;
    }

    err = wait_cts(dev);

    if (err) {
        return err;
    }

    return 0;
}

static int read_response(struct si446x_device *dev, int len, uint8_t *data)
{
    uint8_t resp = 0x00;
    uint16_t attempts = 0;

    //Wait for CTS
    while (resp != 0xFF) {
        digitalWrite(dev->nsel_pin,  LOW);
        SPI.transfer(CMD_READ_CMD_BUF);
        resp = SPI.transfer(0xFF);
        if (resp != 0xFF) {
            digitalWrite(dev->nsel_pin,  HIGH);
            attempts++;
            if (attempts > MAX_CTS_RETRY) {
                return -ETIMEOUT;
            }
            delayMicroseconds(50);
        }
    }

    SPI.transfer(data, len);

    digitalWrite(dev->nsel_pin,  HIGH);
    return 0;
}

static int set_property(struct si446x_device *dev, uint8_t prop_grp,
                        uint8_t prop, uint8_t value)
{
    int err;
    uint8_t buf[] = {
        prop_grp,
        0x01, //length
        prop,
        value
    };

    err = send_command(dev, CMD_SET_PROPERTY, 4, buf);

    if (err) {
        return err;
    }

    return wait_cts(dev);

}

static int get_property(struct si446x_device *dev, uint8_t prop_grp,
                        uint8_t prop, uint8_t *value)
{
    int err;
    uint8_t buf[] = {
        prop_grp,
        0x01, //length
        prop
    };

    err = send_command(dev, CMD_GET_PROPERTY, 3, buf);
    if (err) {
        return err;
    }

    return read_response(dev, 1, value);
}

static int read_rx_fifo(struct si446x_device *dev, int len, uint8_t *data)
{
    if (len < 0 || len > 0xFF) {
        return -EINVAL;
    }

    digitalWrite(dev->nsel_pin,  LOW);
    SPI.transfer(CMD_READ_RX_FIFO);
    SPI.transfer(data, len);
    digitalWrite(dev->nsel_pin,  HIGH);

    return ESUCCESS;
}

static int config_interrupts(struct si446x_device *dev, uint8_t int_gbl,
                             uint8_t int_ph, uint8_t int_modem,
                             uint8_t int_chip)
{
    int err = 0;
    err = set_property(dev, PROP_INT_CTL_GROUP, PROP_INT_CTL_ENABLE, int_gbl);
    if (err) {
        return err;
    }
    err = set_property(dev, PROP_INT_CTL_GROUP, PROP_INT_CTL_PH_ENABLE, int_ph);
    if (err) {
        return err;
    }
    err = set_property(dev, PROP_INT_CTL_GROUP, PROP_INT_CTL_MODEM_ENABLE,
                       int_modem);
    if (err) {
        return err;
    }
    err = set_property(dev, PROP_INT_CTL_GROUP, PROP_INT_CTL_CHIP_ENABLE,
                       int_chip);
    return err;
}

static int write_tx_fifo(struct si446x_device *dev, int len, uint8_t *data)
{
    int err = send_command(dev, CMD_WRITE_TX_FIFO, len, data);

    if (err) {
        return err;
    }

    // TODO: Pretty sure I shouldn't wait for CTS here

    err = wait_cts(dev);

    if (err) {
        return err;
    }

    return ESUCCESS;
}

static int handle_fifo_empty(struct si446x_device *dev)
{
    int rem = dev->tx_buf.len - dev->tx_pkt_index;
    int err = 0;

    if (rem > 0) {
        // Write out next chunk of the packet
        if (rem > FIFO_SIZE - TX_FIFO_THRESH) {
            err = write_tx_fifo(dev, FIFO_SIZE - TX_FIFO_THRESH,
                               dev->tx_buf.data + dev->tx_pkt_index);
            dev->tx_pkt_index += FIFO_SIZE - TX_FIFO_THRESH;
        } else {
            err = write_tx_fifo(dev, rem, dev->tx_buf.data + dev->tx_pkt_index);
            dev->tx_pkt_index += rem;
        }
    }

    return err;
}

static int handle_fifo_full(struct si446x_device *dev)
{
    int err = 0;

    int to_read = RX_FIFO_THRESH;

    if (!dev->has_rx_len) {
        // TODO: Is discarding the volatile qualifier safe here?
        err = read_rx_fifo(dev, sizeof(dev->rx_pkt_len),
                           (uint8_t *) &dev->rx_pkt_len);
        dev->has_rx_len = true;
        to_read -= sizeof(dev->rx_pkt_len);
    }

    if (err) {
        return err;
    }

    int rem = dev->rx_pkt_len - dev->rx_pkt_index;

    if (to_read > rem) {
        // ok, so something is just confused now...
        // Read what should remain then clear the FIFO
        // If the buffer is too small, just discard the packet
        if (dev->rx_pkt_len > dev->rx_buf.len) {
            err = read_rx_fifo(dev, rem, NULL);
        } else {
            err = read_rx_fifo(dev, rem, dev->rx_buf.data + dev->rx_pkt_index);
        }

        dev->rx_pkt_index += rem;

        // Clear the FIFO
        uint8_t fifo_cmd_buf = CLR_RX_FIFO;
        err = send_command(dev, CMD_FIFO_INFO, sizeof(fifo_cmd_buf), &fifo_cmd_buf);

        if (err) {
            return err;
        }

        return wait_cts(dev);

    } else {
        // Read in next chunk of the packet
        // If the buffer is too small, just discard the packet
        if (dev->rx_pkt_len > dev->rx_buf.len) {
            err = read_rx_fifo(dev, to_read, NULL);
        } else {
            err = read_rx_fifo(dev, to_read, dev->rx_buf.data + dev->rx_pkt_index);
        }

        dev->rx_pkt_index += to_read;

        return err;
    }
    // Not reachable
}

static int handle_packet_rx(struct si446x_device *dev)
{

    int err = 0;

    dev->state = IDLE;

    if (!dev->has_rx_len) {
        // TODO: Is discarding the volatile qualifier safe here?
        err = read_rx_fifo(dev, sizeof(dev->rx_pkt_len),
                           (uint8_t *) &dev->rx_pkt_len);
        dev->has_rx_len = true;
    }

    if (err) {
        if (dev->pkt_rx_handler) {
            dev->pkt_rx_handler(dev, err, 0, NULL);
            return ESUCCESS; // We don't report errors twice
        } else {
            return err;
        }
    }

    int rem = dev->rx_pkt_len - dev->rx_pkt_index;

    // Read in whatever remains of the packet
    // If the buffer is too small, just discard the packet
    if (dev->rx_pkt_len > dev->rx_buf.len) {
        err = read_rx_fifo(dev, rem, NULL);
    } else {
        err = read_rx_fifo(dev, rem, dev->rx_buf.data + dev->rx_pkt_index);
    }

    dev->rx_pkt_index += rem;

    if (err) {
        if (dev->pkt_rx_handler) {
            dev->pkt_rx_handler(dev, err, 0, NULL);
            return ESUCCESS; // We don't report errors twice
        } else {
            return err;
        }
    }

    // Packet received, clear whatever remains in the FIFO
    // In case packet handler screwed up...
    uint8_t fifo_cmd_buf = CLR_RX_FIFO;
    err = send_command(dev, CMD_FIFO_INFO, sizeof(fifo_cmd_buf), &fifo_cmd_buf);

    if (err) {
        if (dev->pkt_rx_handler) {
            dev->pkt_rx_handler(dev, err, 0, NULL);
            return ESUCCESS; // We don't report errors twice
        } else {
            return err;
        }
    }

    err = wait_cts(dev);

    if (err && dev->pkt_rx_handler) {
        dev->pkt_rx_handler(dev, err, 0, NULL);
    } else if (dev->rx_pkt_len > dev->rx_buf.len && dev->pkt_rx_handler) {
        dev->pkt_rx_handler(dev, -ETOOLONG, 0, NULL);
    } else if (dev->pkt_rx_handler){
        dev->pkt_rx_handler(dev, ESUCCESS,  dev->rx_pkt_len, dev->rx_buf.data);
    } else if (err) { // Error but no callback registered
        return err;
    }

    return ESUCCESS;
}

static int handle_invalid_chksum(struct si446x_device *dev)
{
    int err = 0;

    dev->state = IDLE;

    if (!dev->has_rx_len) {
        // TODO: Is discarding the volatile qualifier safe here?
        err = read_rx_fifo(dev, sizeof(dev->rx_pkt_len),
                           (uint8_t *) &dev->rx_pkt_len);
        dev->has_rx_len = true;

        if (err) {
            if (dev->pkt_rx_handler) {
                dev->pkt_rx_handler(dev, err, 0, NULL);
                return ESUCCESS; // We don't report errors twice
            } else {
                return err;
            }
        }
    }

    int rem = dev->rx_pkt_len - dev->rx_pkt_index;

    // Read in whatever remains of the packet
    // If the buffer is too small, just discard the packet
    if (dev->rx_pkt_len > dev->rx_buf.len) {
        err = read_rx_fifo(dev, rem, NULL);
    } else {
        err = read_rx_fifo(dev, rem, dev->rx_buf.data + dev->rx_pkt_index);
    }

    // TODO: Figre out a better way to do this...
    // Consider the case where the length byte was corrupt
    // In the event of a checksum miss, we need to purge the FIFO
    uint8_t fifo_cmd_buf = CLR_RX_FIFO;
    err = send_command(dev, CMD_FIFO_INFO, sizeof(fifo_cmd_buf), &fifo_cmd_buf);

    if (err) {
        if (dev->pkt_rx_handler) {
            dev->pkt_rx_handler(dev, err, 0, NULL);
            return ESUCCESS; // We don't report errors twice
        } else {
            return err;
        }
    }

    err = wait_cts(dev);

    dev->rx_pkt_index += rem;

    if (err && dev->pkt_rx_handler) {
        dev->pkt_rx_handler(dev, err, 0, NULL);
    } else if (dev->rx_pkt_len > dev->rx_buf.len && dev->pkt_rx_handler) {
        dev->pkt_rx_handler(dev, -ETOOLONG, 0, NULL);
    } else if (dev->pkt_rx_handler) {
        dev->pkt_rx_handler(dev, -ECHKSUM, dev->rx_pkt_len, dev->rx_buf.data);
    } else if (err) { // Error but no callback registered
        return err;
    }

    return ESUCCESS;
}

static int handle_packet_tx(struct si446x_device *dev)
{

    dev->state = IDLE;

    int err;

    err = set_property(dev, PROP_PKT_GROUP, PROP_PKT_FIELD_2_LENGTH_12_8,
                       ((MAX_PACKET_SIZE) >> 8) & 0xFF);
    if (err) {
      return err;
    }

    err = set_property(dev, PROP_PKT_GROUP, PROP_PKT_FIELD_2_LENGTH_7_0,
                 (MAX_PACKET_SIZE) & 0xFF);
    if (err) {
      return err;
    }

    // TODO: Call the callback with the err if the pkt didn't get sent
    if (dev->pkt_tx_handler) {
        dev->pkt_tx_handler(dev, ESUCCESS);
    }

    return ESUCCESS;
}

// Callback for blocking send
void blocking_tx_complete(struct si446x_device *dev, int err) {
    dev->err = err;
}

// Callback for blocking recv
void blocking_rx_complete(struct si446x_device *dev, int err, int len,
                          uint8_t *data) {
    dev->err = err;
}

int si446x_update(struct si446x_device *dev)
{

    // TODO: RSSI / Preamble / Sync word detect to put us in RX state

    int err = 0;

    // Clear interrupts
    // TODO: struct for this?, use designated initialization
    uint8_t int_data[] = {
        0xC0, // PACKET_SENT_PEND_CLR & PACKET_RX_PEND_CLR & CRC_ERR_PEND_CLR &
              // TX_FIFO_ALMOST_EMPTY_PEND_CLR & RX_FIFO_ALMOST_FULL_PEND_CLR
        0x3F, //
        0xFF  //
    };

    // TODO: Struct for this
    uint8_t int_status[8];

    if (dev->rx_timeout) {
        uint8_t state = STATE_READY;
        err = send_command(dev, CMD_CHANGE_STATE, sizeof(state), &state);

        if (err) {
            return err;
        }

        err = wait_cts(dev);

        if (err) {
            return err;
        }

        uint8_t fifo_cmd_buf = CLR_RX_FIFO;
        err = send_command(dev, CMD_FIFO_INFO, sizeof(fifo_cmd_buf), &fifo_cmd_buf);

        err = wait_cts(dev);

        if (err) {
            return err;
        }

        dev->state = IDLE;
        dev->rx_timeout = false;

        if (dev->pkt_rx_handler) {
            dev->pkt_rx_handler(dev, -ESILICON, 0, NULL);
            return ESUCCESS;
        } else {
            return -ESILICON;
        }
    }

    err = send_command(dev, CMD_GET_INT_STATUS, sizeof(int_data), int_data);

    if (err) {
        return err;
    }

    err = read_response(dev, 8, int_status);

    if (err) {
        return err;
    }

    // TX_FIFO_ALMOST_EMPTY (status, not pending)
    if (int_status[3] & TX_FIFO_ALMOST_EMPTY) {
        err = handle_fifo_empty(dev);
        if (err) {
            return err;
        }
    }

    // RX_FIFO_ALMOST_FULL (status, not pending)
    if (int_status[3] & RX_FIFO_ALMOST_FULL) {
        err = handle_fifo_full(dev);
        if (err) {
            return err;
        }
    }

    // PACKET_RX_PEND
    if (int_status[2] & PACKET_RX) {
        err = handle_packet_rx(dev);
        if (err) {
            return err;
        }
    }

    // CRC_ERR_PEND
    if (int_status[2] & CRC_ERROR) {
        err = handle_invalid_chksum(dev);
        if (err) {
            return err;
        }
    }

    // PACKET_SENT_PEND
    if (int_status[2] & PACKET_SENT) {
        err = handle_packet_tx(dev);
        if (err) {
            return err;
        }
    }

    return ESUCCESS;
}

int si446x_create(struct si446x_device *dev, int nsel_pin, int sdn_pin,
                  int int_pin, uint32_t xo_freq, uint8_t config)
{
    dev->nsel_pin = nsel_pin;
    dev->sdn_pin = sdn_pin;
    dev->xo_freq = xo_freq;
    dev->int_pin = int_pin;

    dev->pkt_tx_handler = NULL;
    dev->tx_buf.data = NULL;
    dev->tx_buf.len = 0;
    dev->tx_pkt_index = 0;

    dev->pkt_rx_handler = NULL;
    dev->rx_pkt_len = 0;
    dev->rx_buf.data = NULL;
    dev->rx_buf.len = 0;
    dev->rx_pkt_index = 0;
    dev->has_rx_len = false;

    dev->config = config;

    dev->rx_timeout = false;

    return 0;
}

int si446x_init(struct si446x_device *dev)
{

    SPI.begin();
    pinMode(dev->int_pin, INPUT_PULLUP);

    pinMode(dev->nsel_pin, OUTPUT);
    digitalWrite(dev->nsel_pin,  HIGH);
    pinMode(dev->sdn_pin, OUTPUT);

    return si446x_reinit(dev);
}

int si446x_reinit(struct si446x_device *dev)
{
    int err;

    //TODO: struct for this?
    uint8_t pwr_up_data[] = {
        BOOT_OPT_NORMAL,
        (byte)(dev->config & 0x01), // XTAL_OPT
        (byte)((dev->xo_freq >> 24) & 0xFF),
        (byte)((dev->xo_freq >> 16) & 0xFF),
        (byte)((dev->xo_freq >>  8) & 0xFF),
        (byte)((dev->xo_freq >>  0) & 0xFF)
    };

    // Reset the Si446x (300 us strobe of SDN)
    digitalWrite(dev->sdn_pin, HIGH);
    delayMicroseconds(300);
    digitalWrite(dev->sdn_pin, LOW);

    // Wait for POR (6 ms)
    delayMicroseconds(6000);

    err = send_command(dev, CMD_POWER_UP, sizeof(pwr_up_data), pwr_up_data);

    if (err) {
        return error(-err, __FILE__, __LINE__);
    }

    err = wait_cts(dev);

    if (err) {
        return error(-err, __FILE__, __LINE__);
    }

    // uint8_t buf_RF_POWER_UP[] = {RF_POWER_UP};
    // err = send_cfg_data_wait(dev, 0x07, buf_RF_POWER_UP);
    // if (err) {
    //      return err;
    // }

    struct si446x_part_info info;
    err = si446x_get_part_info(dev, &info);

    if (err) {
        return error(-err, __FILE__, __LINE__);
    }

    dev->part = info.part;

    if (info.part != 0x4460 && info.part != 0x4461 && info.part != 0x4463 &&
        info.part != 0x4464) {
            return -EWRONGPART;
    }

    // Clear all interrupts
    // TODO: struct  for this?, use designated initialization
    uint8_t int_data[] = {
        0x00,
        0x00,
        0x00
    };

    uint8_t buffer[8];

    err = send_command(dev, CMD_GET_INT_STATUS, sizeof(int_data), int_data);

    if (err) {
        return error(-err, __FILE__, __LINE__);
    }

    err = read_response(dev, 8, buffer);

    if (err) {
        return error(-err, __FILE__, __LINE__);
    }

    // Black-box stuff, send cfg blob
    const uint8_t *cfg = NULL;

    if (dev->part == 0x4463) {
        cfg = si4463_cfg;
    } else if (dev->part == 0x4464) {
        cfg = si4464_cfg;
    } else {
        return -ENOTIMPL;
    }

    int i = 0;
    int len = cfg[i++];
    while (len) {
        err = si446x_send_cfg_data_wait(dev, len, cfg + i);
        if (err) {
             return err;
        }
        i += len;
        len = cfg[i++];
    }

    err = set_property(dev, PROP_PKT_GROUP, PROP_PKT_FIELD_2_LENGTH_12_8,
                 ((MAX_PACKET_SIZE) >> 8) & 0xFF);
    if (err) {
        return error(-err, __FILE__, __LINE__);
    }

    err = set_property(dev, PROP_PKT_GROUP, PROP_PKT_FIELD_2_LENGTH_7_0,
                 (MAX_PACKET_SIZE) & 0xFF);
    if (err) {
        return error(-err, __FILE__, __LINE__);
    }

    err = set_property(dev, PROP_PKT_GROUP, PROP_PKT_TX_THRESHOLD,
                       TX_FIFO_THRESH);
    if (err) {
        return error(-err, __FILE__, __LINE__);
    }

    err = set_property(dev, PROP_PKT_GROUP, PROP_PKT_RX_THRESHOLD,
                       RX_FIFO_THRESH);
    if (err) {
        return error(-err, __FILE__, __LINE__);
    }

    err = config_interrupts(dev, PH_INT_STATUS_EN, PACKET_RX | PACKET_SENT |
                                 CRC_ERROR | TX_FIFO_ALMOST_EMPTY |
                                 RX_FIFO_ALMOST_FULL, 0, 0);
    if (err) {
        return error(-err, __FILE__, __LINE__);
    }

    dev->state = IDLE;

    return ESUCCESS;
}

int si446x_get_part_info(struct si446x_device *device,
                         struct si446x_part_info *info)
{
    uint8_t buf[8];
    int err;

    err = send_command(device, CMD_PART_INFO, 0, NULL);

    if (err) {
        return err;
    }

    err = read_response(device, 8, buf);

    if (err) {
        return err;
    }

    info->chip_rev = buf[0];
    info->part = (buf[1] << 8) | buf[2];
    info->part_build = buf[3];
    info->id = (buf[4] << 8) | buf[5];
    info->customer = buf[6];
    info->rom_id = buf[7];

    return 0;
}

int si446x_set_frequency(struct si446x_device *dev, uint32_t fc, uint8_t band)
{

    set_property(dev, PROP_FREQ_CTRL_GROUP, PROP_FREQ_CTRL_INTE,
                 (fc >> 19) - 1);

    set_property(dev, PROP_FREQ_CTRL_GROUP, PROP_FREQ_CTRL_FRAC_0, fc & 0xFF);
    set_property(dev, PROP_FREQ_CTRL_GROUP, PROP_FREQ_CTRL_FRAC_1,
                 (fc >> 8) & 0xFF);
    set_property(dev, PROP_FREQ_CTRL_GROUP, PROP_FREQ_CTRL_FRAC_2,
                 0x08 | (fc >> 16 & 0x07));

    set_property(dev, PROP_MODEM_GROUP, PROP_MODEM_CLKGEN_BAND, 0x08 | band);

    //TODO: Set FREQ_CONTROL_VCOCNT_RX_ADJ

    return 0;

}

int si446x_config_crc(struct si446x_device *dev, uint8_t cfg)
{
    int err;

    if ((cfg & 0x70) != 0 || (cfg & 0x0F) > 0x08) {
        return -EINVAL;
    }

    err = set_property(dev, PROP_PKT_GROUP, PROP_PKT_CRC_CONFIG, cfg);

    if (err) {
        return err;
    }

    return 0;
}

int si446x_send(struct si446x_device *dev, int len, uint8_t *data)
{
    int err = si446x_send_async(dev, len, data, blocking_tx_complete);

    if (err) {
        return err;
    }

    while (dev->state != IDLE) {
        si446x_update(dev);
        delayMicroseconds(1000);
    }

    return dev->err;
}

int si446x_recv(struct si446x_device *dev, int *len, uint8_t *buf)
{
    int err = si446x_recv_async(dev, *len, buf, blocking_rx_complete);

    if (err) {
        return err;
    }

    while (dev->state != IDLE) {
        si446x_update(dev);
        delayMicroseconds(1000);
    }

    *len = dev->rx_pkt_len;

    return dev->err;
}

int si446x_send_async(struct si446x_device *dev, int len, uint8_t *data,
                      void (*handler)(struct si446x_device *dev, int err))
{
    int err;
    uint8_t packet_len = len;

    if (len < 0) {
        return -EINVAL;
    }

    if (len > MAX_PACKET_SIZE) {
        return -ETOOLONG;
    }

    if (dev->state == RECEIVING || dev->state == TRANSMITTING) {
        return -EBUSY;
    }

    // Clear the both FIFOs
    uint8_t fifo_cmd_buf = CLR_RX_FIFO | CLR_TX_FIFO;
    err = send_command(dev, CMD_FIFO_INFO, sizeof(fifo_cmd_buf), &fifo_cmd_buf);

    if (err) {
        return err;
    }

    err = wait_cts(dev);

    if (err) {
        return err;
    }

    err = send_command(dev, CMD_WRITE_TX_FIFO, sizeof(packet_len), &packet_len);

    if (err) {
        return err;
    }

    dev->pkt_tx_handler = handler;
    dev->tx_buf.data = data;
    dev->tx_buf.len = len;

    if (len > FIFO_INIT_FILL - sizeof(packet_len)) {
        dev->tx_pkt_index = FIFO_INIT_FILL - sizeof(packet_len);
    } else {
        dev->tx_pkt_index = len;
    }

    err = send_command(dev, CMD_WRITE_TX_FIFO, dev->tx_pkt_index, data);

    if (err) {
        return err;
    }

    err = wait_cts(dev);

    if (err) {
        return err;
    }

    uint8_t tx_config[] = {
        0x00, // Channel 0
        0x30, //READY, NO RETRANS, START NOW
        0x00, // length[15:8] (0 = use packet handler)
        0x00, // length[7:0]
    };

    set_property(dev, PROP_PKT_GROUP, PROP_PKT_FIELD_2_LENGTH_12_8,
                 (len >> 8) & 0xFF);
    if (err) {
        return err;
    }

    set_property(dev, PROP_PKT_GROUP, PROP_PKT_FIELD_2_LENGTH_7_0,
                 len & 0xFF);
    if (err) {
        return err;
    }

    // Switch to TUNE_TX
    // This does almost all the work of getting to TX, but helps us avoid an 
    // apparent silicon bug in RFIC. The bug still happens, but only on the 
    // transition into TX. Since the TX_TUNE -> TX transition should only take
    // about 60 us, we can detect this very quickly and reset

    uint8_t next_state = STATE_TUNE_TX;
    err = send_command(dev, CMD_CHANGE_STATE, sizeof(next_state), &next_state);

    if (err) {
        return err;
    }

    err = wait_cts(dev);

    if (err) {
        return err;
    }

    dev->state = TRANSMITTING;

    err = send_command(dev, CMD_START_TX, sizeof(tx_config), tx_config);

    if (err) {
        dev->state = IDLE;
        return err;
    }

    delayMicroseconds(100); // This is the ~60us cited by SiLabs plus margin

    if (poll_cts(dev) != 0xFF) {
        return -ERESETSI;
    }

    return 0;
}

int si446x_recv_async(struct si446x_device *dev, int len,
                      uint8_t *buf,
                      void (*handler)(struct si446x_device *dev, int err,
                                      int len, uint8_t *data))
{

    if (dev->state == RECEIVING || dev->state == TRANSMITTING) {
        return -EBUSY;
    }

    // TODO: This should probably be a critical section with interrupts disabled
    // NULL out the buffer so nothing can fill it while we reset the length
    dev->rx_buf.data = NULL;
    dev->rx_buf.len = len;
    dev->rx_pkt_index = 0;
    dev->has_rx_len = false;
    dev->pkt_rx_handler = handler;
    dev->rx_buf.data = buf;

    int err;

    // Clear the both FIFOs
    uint8_t fifo_cmd_buf = CLR_RX_FIFO | CLR_TX_FIFO;
    err = send_command(dev, CMD_FIFO_INFO, sizeof(fifo_cmd_buf), &fifo_cmd_buf);

    if (err) {
        return err;
    }

    err = wait_cts(dev);

    if (err) {
        return err;
    }

    uint8_t rx_config[] = {
        0x00,               // Channel 0
        0x00,               // RX immediately
        0x00,               // Length[15:8] (0 = use packet handler)
        0x00,               // Length[7:0]
        STATE_RX,           // RX timeout state
        STATE_READY,        // RX valid state
        STATE_READY,        // RX invalid state
    };

    err = send_command(dev, CMD_START_RX, sizeof(rx_config), rx_config);

    if (err) {
        return err;
    }

    err = wait_cts(dev);

    if (err) {
        return err;
    }

    dev->state = LISTEN;

    return ESUCCESS;
}

int si446x_setup_tx(struct si446x_device *dev, int len, uint8_t *data,
                    void (*handler)(struct si446x_device *dev, int err))
{


    int err;
    uint8_t packet_len = len;

    if (len < 0) {
        return -EINVAL;
    }

    if (len > MAX_PACKET_SIZE) {
        return -ETOOLONG;
    }

    err = send_command(dev, CMD_WRITE_TX_FIFO, sizeof(packet_len), &packet_len);

    if (err) {
        return err;
    }

    dev->pkt_tx_handler = handler;
    dev->tx_buf.data = data;
    dev->tx_buf.len = len;

    if (len > FIFO_SIZE - sizeof(packet_len)) {
        dev->tx_pkt_index = FIFO_SIZE - sizeof(packet_len);
    } else {
        dev->tx_pkt_index = len;
    }

    err = send_command(dev, CMD_WRITE_TX_FIFO, dev->tx_pkt_index, data);

    if (err) {
        return err;
    }

    err = wait_cts(dev);

    if (err) {
        return err;
    }

    set_property(dev, PROP_PKT_GROUP, PROP_PKT_FIELD_2_LENGTH_12_8,
                 (len >> 8) & 0xFF);
    if (err) {
        return err;
    }

    set_property(dev, PROP_PKT_GROUP, PROP_PKT_FIELD_2_LENGTH_7_0,
                 len & 0xFF);
    if (err) {
        return err;
    }

    // Switch to TUNE_TX
    // This does almost all the work of getting to TX, but helps us avoid an 
    // apparent silicon bug in RFIC. The bug still happens, but only on the 
    // transition into TX. Since the TX_TUNE -> TX transition should only take
    // about 60 us, we can detect this very quickly and reset

    uint8_t next_state = STATE_TUNE_TX;
    err = send_command(dev, CMD_CHANGE_STATE, sizeof(next_state), &next_state);

    if (err) {
        return err;
    }

    err = wait_cts(dev);

    if (err) {
        return err;
    }

    return 0;
}

int si446x_fire_tx(struct si446x_device *dev)
{
    int err;

    if (dev->state == RECEIVING || dev->state == TRANSMITTING) {
        return -EBUSY;
    }

    uint8_t tx_config[] = {
        0x00, // Channel 0
        0x30, //READY, NO RETRANS, START NOW
        0x00, // length[15:8] (0 = use packet handler)
        0x00, // length[7:0]
    };

    dev->state = TRANSMITTING;

    err = send_command(dev, CMD_START_TX, sizeof(tx_config), tx_config);

    if (err) {
        dev->state = IDLE;
        return err;
    }

    delayMicroseconds(500); // This is the ~60us cited by SiLabs plus margin

    if (poll_cts(dev) != 0xFF) {
        return -ERESETSI;
    }

//    err = set_property(dev, PROP_PKT_GROUP, PROP_PKT_FIELD_2_LENGTH_12_8,
//                 ((MAX_PACKET_SIZE) >> 8) & 0xFF);
//    if (err) {
//      return err;
//    }
//
//    err = set_property(dev, PROP_PKT_GROUP, PROP_PKT_FIELD_2_LENGTH_7_0,
//                 (MAX_PACKET_SIZE) & 0xFF);
//    if (err) {
//      return err;
//    }

//    // Clear the RX FIFO
//    uint8_t fifo_cmd_buf = CLR_RX_FIFO;
//    err = send_command(dev, CMD_FIFO_INFO, sizeof(fifo_cmd_buf), &fifo_cmd_buf);
//
//    if (err) {
//        return err;
//    }
//
//    err = wait_cts(dev);
//
//    if (err) {
//        return err;
//    }

    return 0;
}

int si446x_idle(struct si446x_device *dev)
{
    int err;
    enum si446x_state old_state = dev->state;

    dev->state = IDLE;

    uint8_t next_state = STATE_SLEEP;
    err = send_command(dev, CMD_CHANGE_STATE, sizeof(next_state), &next_state);

    if (err) {
        dev->state = old_state;
        return err;
    }

    err = wait_cts(dev);

    if (err) {
        dev->state = old_state;
        return err;
    }

    next_state = STATE_READY;
    err = send_command(dev, CMD_CHANGE_STATE, sizeof(next_state), &next_state);

    if (err) {
        dev->state = IDLE;
        return err;
    }

    err = wait_cts(dev);

    if (err) {
        dev->state = IDLE;
        return err;
    }

    return 0;
}

int si446x_set_tx_pwr(struct si446x_device *dev, uint8_t pwr)
{
    return set_property(dev, PROP_PA_GROUP, PROP_PA_PWR_LVL, pwr);
}

int si446x_cfg_gpio(struct si446x_device *dev, uint8_t gpio0, uint8_t gpio1,
                    uint8_t gpio2, uint8_t gpio3)
{
    uint8_t gpio_cfg[] = {
        gpio0,
        gpio1,
        gpio2,
        gpio3,
        0x00, // NIRQ
        0x00, // SDO
        0x00, // Drive strength (strongest)
    };

    int err = send_command(dev, CMD_GPIO_PIN_CFG, sizeof(gpio_cfg), gpio_cfg);

    if (err) {
        return err;
    }

    return wait_cts(dev);
}

int si446x_set_mod_type(struct si446x_device *dev, uint8_t mod_type) {
    return set_property(dev, PROP_MODEM_GROUP, PROP_MODEM_MOD_TYPE, mod_type);
}

int si446x_check_crc(struct si446x_device *dev, bool check_crc) {

    int err;
    uint8_t crc_cfg;

    err = get_property(dev, PROP_PKT_GROUP, PROP_PKT_FIELD_2_CRC_CONFIG, &crc_cfg);
    if (err) {
        return err;
    }

    if (check_crc) {
        crc_cfg |= 0x08;  // CHECK_CRC
    } else {
        crc_cfg &= ~0x08; // ~CHECK_CRC
    }

    return set_property(dev, PROP_PKT_GROUP, PROP_PKT_FIELD_2_CRC_CONFIG, crc_cfg);
}

int si446x_data_whitening(struct si446x_device *dev, bool whiten) {

    int err;
    uint8_t pkt_config;

    // Enable whitening for field 1 (length byte)
    err = get_property(dev, PROP_PKT_GROUP, PROP_PKT_FIELD_1_CONFIG, &pkt_config);
    if (err) {
        return err;
    }

    if (whiten) {
        pkt_config |= 0x06;  // WHITEN | PN_START
    } else {
        pkt_config &= ~0x06; // ~WHITEN | PN_START
    }

    err = set_property(dev, PROP_PKT_GROUP, PROP_PKT_FIELD_1_CONFIG, pkt_config);
    if (err) {
        return err;
    }

    // Enable whitening for field 2 (payload)
    err = get_property(dev, PROP_PKT_GROUP, PROP_PKT_FIELD_2_CONFIG, &pkt_config);
    if (err) {
        return err;
    }

    if (whiten) {
        pkt_config |= 0x02;  // WHITEN
    } else {
        pkt_config &= ~0x02; // ~WHITEN
    }

    err = set_property(dev, PROP_PKT_GROUP, PROP_PKT_FIELD_2_CONFIG, pkt_config);
    if (err) {
        return err;
    }

    return ESUCCESS;
}

int si446x_rx_timeout(struct si446x_device *dev)
{
    dev->rx_timeout = true;
    return ESUCCESS;
}

int si446x_set_deviation(struct si446x_device *dev, uint32_t deviation) {

    int err;

    if (deviation > 0x0001FFFF) {
        return -EINVAL;
    }

    err = set_property(dev, PROP_MODEM_GROUP, PROP_MODEM_FREQ_DEV_2,
                (deviation >> 16) & 0xFF);

    if (err) {
        return err;
    }

    err = set_property(dev, PROP_MODEM_GROUP, PROP_MODEM_FREQ_DEV_1,
                (deviation >> 8) & 0xFF);

    if (err) {
        return err;
    }

    return set_property(dev, PROP_MODEM_GROUP, PROP_MODEM_FREQ_DEV_0, 
                (deviation >> 0) & 0xFF);
}

int si446x_set_freq_offset(struct si446x_device *dev, uint16_t offset_counts) {
    int err;

    uint8_t buf[] = {
        PROP_MODEM_GROUP,
        0x02, //length
        PROP_MODEM_FREQ_OFFSET_1,
        (byte)((offset_counts >> 8) & 0xFF),
        (byte)((offset_counts >> 0) & 0xFF)
    };

    err = send_command(dev, CMD_SET_PROPERTY, sizeof(buf), buf);

    if (err) {
        return err;
    }

    return wait_cts(dev);
}

int si446x_read_rssi(struct si446x_device *dev, uint8_t *rssi, uint8_t *rssi_latch) {

    int err;
    uint8_t cmd_buf = 0xFF;
    uint8_t resp_buf[4] = {0};
    uint8_t comp = 0;

    err = send_command(dev, CMD_GET_MODEM_STATUS, sizeof(cmd_buf), &cmd_buf);

    if (err) {
        return err;
    }

    err = read_response(dev, 4, resp_buf);

    if (err) {
        return err;
    }

    get_property(dev, PROP_MODEM_GROUP, MODEM_RSSI_COMP, &comp);

    if (rssi) {
        *rssi = resp_buf[2] + (comp - 0x40);
    }

    if (rssi_latch) {
        *rssi_latch = resp_buf[3] + (comp - 0x40);
    }

    return ESUCCESS;
}


//int si446x_get_temp(struct si446x_device *dev, int *temp) {
//
//    int err;
//
//    *temp = 0;
//
//    uint8_t adc_config[] = {
//        0x10 // TEMP only
//    };
//
//    uint8_t adc_results[]
//
//    send_command(dev, CMD_START_TX, sizeof(tx_config), tx_config);
//}
