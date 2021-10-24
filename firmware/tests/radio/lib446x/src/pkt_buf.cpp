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

#include <string.h>

#include "pkt_buf.h"
#include "error.h"

int pkt_buf_init(struct pkt_buf *buf, int capacity, uint8_t *buffer)
{
    if (capacity <= 0 || capacity > 0xFFFF) {
        return -EINVAL;
    }

    buf->capacity = capacity;
    buf->base = buffer;
    buf->head = buffer;
    buf->tail = buffer;
    buf->depth = 0;

    return ESUCCESS;
}

int pkt_buf_enqueue(struct pkt_buf *buf, int len, uint8_t *data)
{
    if (len <= 0 || len > 0xFFFF) {
        return -EINVAL;
    }

    int avail = buf->capacity - pkt_buf_size(buf);
    if (len > (avail - 2)) {
        return -EOVERFLOW;
    }

    // Can we fit the entire packet in one contiguous chunk?
    int rem = buf->capacity - (buf->head - buf->base);
    if (rem < len + 2) {
        // Split the packet and wrap around

        // MSB of length
        *buf->head = len >> 8;
        rem--;
        if (rem > 0) {
            buf->head++;
        } else {
            buf->head = buf->base;
            rem = buf->capacity;
        }

        // LSB of length
        *buf->head = len & 0xFF;
        rem--;
        if (rem > 0) {
            buf->head++;
        } else {
            buf->head = buf->base;
            rem = buf->capacity;
        }

        if (rem > len) {
            memcpy(buf->head, data, len);
            buf->head += len;
        } else {
            memcpy(buf->head, data, rem);
            buf->head = buf->base;
            memcpy(buf->head, data + rem, len - rem);
            buf->head += len - rem;
        }
    } else {
        // MSB of length
        *buf->head = len >> 8;
        // LSB of length
        *(buf->head+1) = len & 0xFF;
        buf->head += 2;
        // Data
        memcpy(buf->head, data, len);
        buf->head += len;
        if (buf->head == buf->base + buf->capacity) {
            buf->head = buf->base;
        }
    }

    buf->depth++;

    return ESUCCESS;
}

int pkt_buf_dequeue(struct pkt_buf *buf, int *len, uint8_t *data)
{
    if (*len <= 0) {
        return -EINVAL;
    }

    if (pkt_buf_size(buf) == 0) {
        return -EUNDERFLOW;
    }

    uint8_t *old_tail = buf->tail;

    // Did we fit the entire packet in one contiguous chunk?
    uint16_t rem = buf->capacity - (buf->tail - buf->base);
    
    uint16_t pkt_len = (*buf->tail) << 8;
    rem--;

    if (rem > 0) {
        buf->tail++;
    } else {
        buf->tail = buf->base;
        rem = buf->capacity;
    }

    pkt_len |= *buf->tail;
    rem--;

    if (rem > 0) {
        buf->tail++;
    } else {
        buf->tail = buf->base;
        rem = buf->capacity;
    }

    if (pkt_len > *len) {
        buf->tail = old_tail;
        return -ETOOLONG;
    }

    if (rem <= pkt_len) {
        memcpy(data, buf->tail, rem);
        buf->tail = buf->base;
        memcpy(data + rem, buf->tail, pkt_len - rem);
        buf->tail += pkt_len - rem;
    } else {
        memcpy(data, buf->tail, pkt_len);
        buf->tail += pkt_len;
    }

    buf->depth -= 1;
    *len = pkt_len;

    return ESUCCESS;
}

void pkt_buf_clear(struct pkt_buf *buf)
{
    buf->head = buf->base;
    buf->tail = buf->base;
    buf->depth = 0;
}

int pkt_buf_depth(struct pkt_buf *buf)
{
    return buf->depth;
}

int pkt_buf_size(struct pkt_buf *buf)
{
    if (buf->head >= buf->tail) {
        return buf->head - buf->tail;
    } else {
        return buf->capacity - (buf->tail - buf->head);
    }
}
