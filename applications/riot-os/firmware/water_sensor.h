/*
 * Copyright (C) 2021 Bas Stottelaar <basstottelaar@gmail.com>
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @defgroup    drivers_water_sensor Water level sensor
 * @ingroup     drivers_sensors
 * @brief       Driver for the water level sensor
 *
 * @{
 *
 * @file
 * @brief       Interface definition of the Si70xx driver.
 *
 * @author      Bas Stottelaar <basstottelaar@gmail.com>
 */

#ifndef WATER_SENSOR_H
#define WATER_SENSOR_H

#include <stdint.h>
#include <stdbool.h>

#include "periph/i2c.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Driver return codes
 */
enum {
    WATER_SENSOR_OK,                 /**< All OK */
    WATER_SENSOR_ERR_NODEV,          /**< No valid device found on I2C bus */
    WATER_SENSOR_ERR_I2C,            /**< An error occurred when reading/writing on I2C bus */
};

/**
 * @brief Device initialization parameters.
 */
typedef struct {
    i2c_t i2c_dev;              /**< I2C bus the sensor is connected to */
    uint8_t address;            /**< sensor address */
} water_sensor_params_t;

/**
 * @brief Si70xx device descriptor.
 */
typedef struct {
    water_sensor_params_t params;     /**< Device parameters */
} water_sensor_t;

typedef struct {
    uint8_t id;
    uint8_t level_channels;
    uint8_t temperature_channels;
    bool enabled;
    uint8_t errors;
    uint8_t context;
} water_sensor_info_t;

typedef struct {
    int16_t value;
    int8_t channel;
    bool valid;
} water_sensor_level_t;

typedef struct {
    int16_t value;
    int8_t channel;
    bool valid;
} water_sensor_temperature_t;

typedef struct {
    uint16_t value;
    uint16_t min;
    uint16_t max;
    bool valid;
} water_sensor_level_raw_t;

typedef struct {
    int16_t value;
    int16_t min;
    int16_t max;
    bool valid;
} water_sensor_temperature_raw_t;

typedef struct {
    int16_t default_level;
} water_sensor_config_t;

typedef struct {
    bool enabled;
    uint16_t samples;
    uint8_t alpha;
    uint16_t offset;
    int16_t level;
} water_sensor_level_config_t;

typedef struct {
    bool enabled;
    uint8_t alpha;
    uint16_t reference;
} water_sensor_temperature_config_t;

int water_sensor_init(water_sensor_t *dev, const water_sensor_params_t *params);
int water_sensor_reset(const water_sensor_t *dev);
int water_sensor_enable(const water_sensor_t *dev);
int water_sensor_load(const water_sensor_t *dev);
int water_sensor_store(const water_sensor_t *dev);
int water_sensor_calibrate(const water_sensor_t *dev);
int water_sensor_zero(const water_sensor_t *dev);
int water_sensor_read_info(const water_sensor_t *dev, water_sensor_info_t *out);
int water_sensor_read_level(const water_sensor_t *dev, water_sensor_level_t *out);
int water_sensor_read_temperature(const water_sensor_t *dev, water_sensor_temperature_t *out);
int water_sensor_read_level_raw(const water_sensor_t *dev, uint8_t channel, water_sensor_level_raw_t *out);
int water_sensor_read_temperature_raw(const water_sensor_t *dev, uint8_t channel, water_sensor_temperature_raw_t *out);
int water_sensor_read_config(const water_sensor_t *dev, water_sensor_config_t *out);
int water_sensor_write_config(const water_sensor_t *dev, const water_sensor_config_t *in);
int water_sensor_read_level_config(const water_sensor_t *dev, uint8_t channel, water_sensor_level_config_t *out);
int water_sensor_write_level_config(const water_sensor_t *dev, uint8_t channel, const water_sensor_level_config_t *in);
int water_sensor_read_temperature_config(const water_sensor_t *dev, uint8_t channel, water_sensor_temperature_config_t *out);
int water_sensor_write_temperature_config(const water_sensor_t *dev, uint8_t channel, const water_sensor_temperature_config_t *in);

#ifdef __cplusplus
}
#endif

#endif /* WATER_SENSOR_H */
/** @} */
