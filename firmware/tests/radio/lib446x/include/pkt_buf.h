/* Little Free Radio - An Open Source Radio for CubeSats
 * Copyright (C) 2018 Grant Iraci, Brian Bezanson
 * A project of the University at Buffalo Nanosatellite Laboratory
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef PKT_BUF_H
#define PKT_BUF_H

#include <stdint.h>

struct pkt_buf
{
    uint16_t capacity;  // Size of backing buffer
    uint8_t *head;      // Pointer to first unallocated byte
    uint8_t *tail;      // Pointer to first allocated byte
    uint8_t *base;      // Pointer to base of backing buffer
    uint16_t depth;    // Number of packets in the buffer
};

/**
 * Initialize a new packet buffer structure
 * @param buf the buffer structure to initialize
 * @param capacity the number of bytes in the backing buffer
 * @param buffer pointer to the beginning of the backing buffer
 * @return negative error code or 0 for success
 */
int pkt_buf_init(struct pkt_buf *buf, int capacity, uint8_t *buffer);


/**
 * Add a packet to the buffer
 * @param buf the buffer structure
 * @param len the number of bytes in the packet
 * @param data pointer to packet data
 * @return negative error code or 0 for success
 */
int pkt_buf_enqueue(struct pkt_buf *buf, int len, uint8_t *data);

/**
 * Remove a packet from the buffer
 * @param buf the buffer structure
 * @param len maximium size packet to dequeue, overwritten with 
 *            actual packet size
 * @param data pointer to buffer to fill with packet data
 * @return negative error code or 0 for success
 */
int pkt_buf_dequeue(struct pkt_buf *buf, int *len, uint8_t *data);

/**
 * Clear the buffer, dropping all packets
 * @param buf the buffer structure
 * @return negative error code or 0 for success
 */
void pkt_buf_clear(struct pkt_buf *buf);

/**
 * Get the number of packets in the buffer
 * @param buf the buffer structure
 * @return number of packets
 */
int pkt_buf_depth(struct pkt_buf *buf);

/**
 * Get the number of used bytes in the buffer
 * @param buf the buffer structure
 * @return number of bytes
 */
int pkt_buf_size(struct pkt_buf *buf);

#endif
