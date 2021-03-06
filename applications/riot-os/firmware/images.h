/*
 * Copyright (C) 2021 Bas Stottelaar <basstottelaar@gmail.com>
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

#ifndef IMAGES_H
#define IMAGES_H

#include "stdint.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief   Image of relative humidity (16x16 pixels).
 */
const uint8_t image_rh [] = {
    0x00, 0x40, 0x00, 0xE0, 0x02, 0xA0, 0x05, 0x10, 0x05, 0x08, 0x08, 0x88,
    0x10, 0x44, 0x10, 0x44, 0x20, 0x24, 0x20, 0x24, 0x28, 0x24, 0x28, 0x24,
    0x28, 0x28, 0x20, 0x30, 0x10, 0x40, 0x0F, 0x80
};

/**
 * @brief   Image of temperature (16x16 pixels).
 */
const uint8_t image_temperature [] = {
    0x03, 0x00, 0x04, 0x80, 0x04, 0x80, 0x04, 0x80, 0x04, 0x80, 0x04, 0x80,
    0x04, 0x80, 0x04, 0x80, 0x04, 0x80, 0x08, 0x40, 0x17, 0xA0, 0x17, 0xA0,
    0x17, 0xA0, 0x17, 0xA0, 0x08, 0x40, 0x07, 0x80
};

#ifdef __cplusplus
}
#endif

#endif /* IMAGES_H */
