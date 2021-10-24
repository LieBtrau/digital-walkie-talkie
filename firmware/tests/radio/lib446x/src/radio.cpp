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

#include <stdbool.h>
#include <stdio.h>
#include "si446x.h"
#include "radio.h"
#include "modem_configs.h"

extern struct si446x_device dev;

int set_frequency(uint32_t freq)
{
    uint8_t outdiv;
    uint8_t band;
    int err;

    struct si446x_part_info info;

    err = si446x_get_part_info(&dev, &info);

    if (err) {
        return err;
    }

    if (info.part == 0x4464) {
        if (freq > 119000000 && freq < 159000000) {
            outdiv = 24;
            band = 5;
        } else if (freq > 177000000 && freq < 239000000) {
            outdiv = 16;
            band = 4;
        } else if (freq > 235000000 && freq < 319000000) {
            outdiv = 12;
            band = 3;
        } else if (freq > 353000000 && freq < 479000000) {
            outdiv = 8;
            band = 2;
        } else if (freq > 470000000 && freq < 639000000) {
            outdiv = 6;
            band = 1;
        } else if (freq > 705000000 && freq < 960000000) {
            outdiv = 4;
            band = 0;
        }  else {
            return -EINVAL;
        }
    } else { // 4460, 4461, 4463
        if (freq > 142000000 && freq < 175000000) {
            outdiv = 24;
            band = 5;
        } else if (freq > 284000000 && freq < 350000000) {
            outdiv = 12;
            band = 3;
        } else if (freq > 420000000 && freq < 525000000) {
            outdiv = 8;
            band = 2;
        } else if (freq > 850000000 && freq < 1050000000) {
            outdiv = 4;
            band = 0;
        }  else {
            return -EINVAL;
        }
    }

    // fc = F / (2 * xo_freq / outdiv) * 2^19
    uint32_t fc = (uint32_t) (outdiv * ((uint64_t) freq << 18) / XTAL_FREQ);

    return si446x_set_frequency(&dev, fc, band);
}

int set_modem_config(uint8_t cfg)
{

    static const struct modem_cfg cfg_5k_data_1k25_dev_struct = CFG_DATA_5K_DEV_1K25;
    static const struct modem_cfg cfg_5k_data_2k5_dev_struct = CFG_DATA_5K_DEV_2K5;
    static const struct modem_cfg cfg_10k_data_2k5_dev_struct = CFG_DATA_10K_DEV_2K5;
    static const struct modem_cfg cfg_10k_data_5k_dev_struct = CFG_DATA_10K_DEV_5K;
    static const struct modem_cfg cfg_25k_data_6k25_dev_struct = CFG_DATA_25K_DEV_6K25;
    static const struct modem_cfg cfg_25k_data_12k5_dev_struct = CFG_DATA_25K_DEV_12K5;
    static const struct modem_cfg cfg_50k_data_12k5_dev_struct = CFG_DATA_50K_DEV_12K5;
    static const struct modem_cfg cfg_50k_data_25k_dev_struct = CFG_DATA_50K_DEV_25K;
    static const struct modem_cfg cfg_100k_data_25k_dev_struct = CFG_DATA_100K_DEV_25K;
    static const struct modem_cfg cfg_100k_data_50k_dev_struct = CFG_DATA_100K_DEV_50K;

    const struct modem_cfg *cfg_struct;

    switch (cfg) {
    case DATA_5K_DEV_1K25:
        cfg_struct = &cfg_5k_data_1k25_dev_struct;
        break;

    case DATA_5K_DEV_2K5:
        cfg_struct = &cfg_5k_data_2k5_dev_struct;
        break;

    case DATA_10K_DEV_2K5:
        cfg_struct = &cfg_10k_data_2k5_dev_struct;
        break;

    case DATA_10K_DEV_5K:
        cfg_struct = &cfg_10k_data_5k_dev_struct;
        break;

    case DATA_25K_DEV_6K25:
        cfg_struct = &cfg_25k_data_6k25_dev_struct;
        break;

    case DATA_25K_DEV_12K5:
        cfg_struct = &cfg_25k_data_12k5_dev_struct;
        break;

    case DATA_50K_DEV_12K5:
        cfg_struct = &cfg_50k_data_12k5_dev_struct;
        break;

    case DATA_50K_DEV_25K:
        cfg_struct = &cfg_50k_data_25k_dev_struct;
        break;

    case DATA_100K_DEV_25K:
        cfg_struct = &cfg_100k_data_25k_dev_struct;
        break;

    case DATA_100K_DEV_50K:
        cfg_struct = &cfg_100k_data_50k_dev_struct;
        break;


    default:
        if (cfg & 0xF0) {
            return -EINVAL;
        } else {
            return -ENOTIMPL;
        }
    }

    int err;

    err = si446x_send_cfg_data_wait(&dev,
                              sizeof(cfg_struct->modem_mod_type_12),
                              cfg_struct->modem_mod_type_12);
    if (err) {
        return err;
    }

    err = si446x_send_cfg_data_wait(&dev,
                              sizeof(cfg_struct->modem_freq_dev_0_1),
                              cfg_struct->modem_freq_dev_0_1);
    if (err) {
        return err;
    }

    err = si446x_send_cfg_data_wait(&dev,
                              sizeof(cfg_struct->modem_tx_ramp_delay_8),
                              cfg_struct->modem_tx_ramp_delay_8);
    if (err) {
        return err;
    }

    err = si446x_send_cfg_data_wait(&dev,
                                sizeof(cfg_struct->modem_bcr_osr_1_9),
                                cfg_struct->modem_bcr_osr_1_9);
    if (err) {
        return err;
    }

    err = si446x_send_cfg_data_wait(&dev,
                                sizeof(cfg_struct->modem_afc_gear),
                                cfg_struct->modem_afc_gear);
    if (err) {
        return err;
    }

    err = si446x_send_cfg_data_wait(&dev,
                                sizeof(cfg_struct->modem_agc_window_size_9),
                                cfg_struct->modem_agc_window_size_9);
    if (err) {
        return err;
    }

    err = si446x_send_cfg_data_wait(&dev,
                                sizeof(cfg_struct->modem_ook_cnt1_9),
                                cfg_struct->modem_ook_cnt1_9);
    if (err) {
        return err;
    }

    err = si446x_send_cfg_data_wait(&dev,
                                sizeof(cfg_struct->modem_chflt_coe13_7_0_12),
                                cfg_struct->modem_chflt_coe13_7_0_12);
    if (err) {
        return err;
    }

    err = si446x_send_cfg_data_wait(&dev,
                                sizeof(cfg_struct->modem_chflt_coe1_7_0_12),
                                cfg_struct->modem_chflt_coe1_7_0_12);
    if (err) {
        return err;
    }

    err = si446x_send_cfg_data_wait(&dev,
                                sizeof(cfg_struct->modem_chflt_coe7_7_0_12),
                                cfg_struct->modem_chflt_coe7_7_0_12);
    if (err) {
        return err;
    }

    err = si446x_send_cfg_data_wait(&dev,
                                sizeof(cfg_struct->rf_synth_pfdcp_cpff),
                                cfg_struct->rf_synth_pfdcp_cpff);

    return err;

}


