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

#pragma once

#include <stdbool.h>
#include <stdint.h>

#define XTAL_FREQ   26000000L


#define DATA_1K_DEV_0K25        0x00
#define DATA_1K_DEV_0K5         0x01
#define DATA_2K5_DEV_0K625      0x02
#define DATA_2K5_DEV_1K25       0x03
#define DATA_5K_DEV_1K25        0x04
#define DATA_5K_DEV_2K5         0x05
#define DATA_10K_DEV_2K5        0x06
#define DATA_10K_DEV_5K         0x07
#define DATA_25K_DEV_6K25       0x08
#define DATA_25K_DEV_12K5       0x09
#define DATA_50K_DEV_12K5       0x0A
#define DATA_50K_DEV_25K        0x0B
#define DATA_100K_DEV_25K       0x0C
#define DATA_100K_DEV_50K       0x0D
#define DATA_250K_DEV_62K5      0x0E
#define DATA_250K_DEV_125K      0x0F

int set_frequency(uint32_t freq);
int set_modem_config(uint8_t cfg);

