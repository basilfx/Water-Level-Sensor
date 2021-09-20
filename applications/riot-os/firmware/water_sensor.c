/*
 * Copyright (C) 2021 Bas Stottelaar <basstottelaar@gmail.com>
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     drivers_water_sensor
 * @{
 *
 * @file
 * @brief       Implementation of the water level sensor.
 *
 * @author      Bas Stottelaar <basstottelaar@gmail.com>
 *
 * @}
 */

#include "water_sensor.h"
#include "water_sensor_internals.h"

#define ENABLE_DEBUG 0
#include "debug.h"

#define WATER_SENSOR_I2C     (dev->params.i2c_dev)
#define WATER_SENSOR_ADDR    (dev->params.address)

static uint8_t _checksum(const uint8_t *data, size_t length)
{
    uint8_t checksum = 0xff;

    for (unsigned i = 0; i < length; i++) {
        checksum ^= data[i];
    }

    return checksum;
}

static int _cmd(const water_sensor_t *dev, uint8_t cmd)
{
    int result;

    i2c_acquire(WATER_SENSOR_I2C);
    result = i2c_write_byte(WATER_SENSOR_I2C, WATER_SENSOR_ADDR, cmd, 0);
    i2c_release(WATER_SENSOR_I2C);

    return result;
}

static int _read_reg(const water_sensor_t *dev, uint8_t reg, void *data, size_t length)
{
    int result;

    i2c_acquire(WATER_SENSOR_I2C);
    result = i2c_read_regs(WATER_SENSOR_I2C, WATER_SENSOR_ADDR, reg, data, length, 0);
    i2c_release(WATER_SENSOR_I2C);

    return result;
}

static int _read_reg16(const water_sensor_t *dev, uint16_t reg, void *data, size_t length)
{
    int result;

    i2c_acquire(WATER_SENSOR_I2C);
    result = i2c_read_regs(WATER_SENSOR_I2C, WATER_SENSOR_ADDR, reg, data, length, I2C_REG16);
    i2c_release(WATER_SENSOR_I2C);

    return result;
}

static int _write_reg(const water_sensor_t *dev, uint8_t reg, const void *data, size_t length)
{
    int result;

    i2c_acquire(WATER_SENSOR_I2C);
    result = i2c_write_regs(WATER_SENSOR_I2C, WATER_SENSOR_ADDR, reg, data, length, 0);
    i2c_release(WATER_SENSOR_I2C);

    return result;
}

static int _write_reg16(const water_sensor_t *dev, uint16_t reg, const void *data, size_t length)
{
    int result;

    i2c_acquire(WATER_SENSOR_I2C);
    result = i2c_write_regs(WATER_SENSOR_I2C, WATER_SENSOR_ADDR, reg, data, length, I2C_REG16);
    i2c_release(WATER_SENSOR_I2C);

    return result;
}

int water_sensor_init(water_sensor_t *dev, const water_sensor_params_t *params)
{
    /* initialize the device descriptor */
    dev->params = *params;

    /* reset the device */
    if (water_sensor_reset(dev) != WATER_SENSOR_OK) {
        DEBUG("[water_sensor] water_sensor_init: reset failed\n");
        return WATER_SENSOR_ERR_I2C;
    }

    /* read sensor identification */
    water_sensor_info_t info;

    if (water_sensor_read_info(dev, &info) != WATER_SENSOR_OK) {
        DEBUG("[water_sensor] water_sensor_init: info failed\n");
        return WATER_SENSOR_ERR_I2C;
    }

    if (info.id != WATER_SENSOR_ID) {
        DEBUG("[water_sensor] water_sensor_init: identification failed\n");
        return WATER_SENSOR_ERR_NODEV;
    }

    if (info.errors != 0) {
        DEBUG("[water_sensor] water_sensor_init: sensor errors is %02x (context %d)\n", info.errors, info.context);
        return WATER_SENSOR_ERR_NODEV;
    }

    return WATER_SENSOR_OK;
}

int water_sensor_reset(const water_sensor_t *dev)
{
    if (_cmd(dev, WATER_SENSOR_RESET) != 0) {
        DEBUG("[water_sensor] water_sensor_reset: failed\n");
        return WATER_SENSOR_ERR_I2C;
    }

    return WATER_SENSOR_OK;
}

int water_sensor_enable(const water_sensor_t *dev)
{
    if (_cmd(dev, WATER_SENSOR_ENABLE) != 0) {
        DEBUG("[water_sensor] water_sensor_enable: failed\n");
        return WATER_SENSOR_ERR_I2C;
    }

    return WATER_SENSOR_OK;
}

int water_sensor_load(const water_sensor_t *dev)
{
    if (_cmd(dev, WATER_SENSOR_LOAD) != 0) {
        DEBUG("[water_sensor] water_sensor_load: failed\n");
        return WATER_SENSOR_ERR_I2C;
    }

    return WATER_SENSOR_OK;
}

int water_sensor_store(const water_sensor_t *dev)
{
    if (_cmd(dev, WATER_SENSOR_STORE) != 0) {
        DEBUG("[water_sensor] water_sensor_store: failed\n");
        return WATER_SENSOR_ERR_I2C;
    }

    return WATER_SENSOR_OK;
}

int water_sensor_calibrate(const water_sensor_t *dev)
{
    if (_cmd(dev, WATER_SENSOR_CALIBRATE) != 0) {
        DEBUG("[water_sensor] water_sensor_calibrate: failed\n");
        return WATER_SENSOR_ERR_I2C;
    }

    return WATER_SENSOR_OK;
}

int water_sensor_zero(const water_sensor_t *dev)
{
    if (_cmd(dev, WATER_SENSOR_ZERO) != 0) {
        DEBUG("[water_sensor] water_sensor_zero: failed\n");
        return WATER_SENSOR_ERR_I2C;
    }

    return WATER_SENSOR_OK;
}

int water_sensor_read_info(const water_sensor_t *dev, water_sensor_info_t *out)
{
    assert(out != NULL);

    uint8_t buf[WATER_SENSOR_INFO_SIZE + 1];

    if (_read_reg(dev, WATER_SENSOR_READ_INFO, buf, sizeof(buf)) != 0) {
        DEBUG("[water_sensor] water_sensor_read_info: failed\n");
        return WATER_SENSOR_ERR_I2C;
    }

    if (_checksum(buf, WATER_SENSOR_INFO_SIZE) != buf[6]) {
        DEBUG("[water_sensor] water_sensor_read_info: checksum error\n");
        return WATER_SENSOR_ERR_I2C;
    }

    out->id = buf[0];
    out->level_channels = buf[1];
    out->temperature_channels = buf[2];
    out->enabled = buf[3];
    out->errors = buf[4];
    out->context = buf[5];

    return WATER_SENSOR_OK;
}

int water_sensor_read_level(const water_sensor_t *dev, water_sensor_level_t *out)
{
    assert(out != NULL);

    uint8_t buf[WATER_SENSOR_LEVEL_SIZE + 1];

    if (_read_reg(dev, WATER_SENSOR_READ_LEVEL, buf, sizeof(buf)) != 0) {
        DEBUG("[water_sensor] water_sensor_read_level: failed\n");
        return WATER_SENSOR_ERR_I2C;
    }

    if (_checksum(buf, WATER_SENSOR_TEMPERATURE_SIZE) != buf[4]) {
        DEBUG("[water_sensor] water_sensor_read_level: checksum error\n");
        return WATER_SENSOR_ERR_I2C;
    }

    out->value = (buf[0] << 8) | buf[1];
    out->channel = buf[2];
    out->valid = buf[3] != 0;

    return WATER_SENSOR_OK;
}

int water_sensor_read_temperature(const water_sensor_t *dev, water_sensor_temperature_t *out)
{
    assert(out != NULL);

    uint8_t buf[WATER_SENSOR_TEMPERATURE_SIZE + 1];

    if (_read_reg(dev, WATER_SENSOR_READ_TEMPERATURE, buf, sizeof(buf)) != 0) {
        DEBUG("[water_sensor] water_sensor_read_temperature: failed\n");
        return WATER_SENSOR_ERR_I2C;
    }

    if (_checksum(buf, WATER_SENSOR_TEMPERATURE_SIZE) != buf[4]) {
        DEBUG("[water_sensor] water_sensor_read_temperature: checksum error\n");
        return WATER_SENSOR_ERR_I2C;
    }

    out->value = (buf[0] << 8) | buf[1];
    out->channel = buf[2];
    out->valid = buf[3] != 0;

    return WATER_SENSOR_OK;
}

int water_sensor_read_level_raw(const water_sensor_t *dev, uint8_t channel, water_sensor_level_raw_t *out)
{
    assert(out != NULL);

    uint8_t buf[WATER_SENSOR_LEVEL_RAW_SIZE + 1];
    uint16_t reg = (WATER_SENSOR_READ_LEVEL_RAW << 8) | channel;

    if (_read_reg16(dev, reg, buf, sizeof(buf)) != 0) {
        DEBUG("[water_sensor] water_sensor_read_level_raw: failed\n");
        return WATER_SENSOR_ERR_I2C;
    }

    if (_checksum(buf, WATER_SENSOR_LEVEL_RAW_SIZE) != buf[7]) {
        DEBUG("[water_sensor] water_sensor_read_level_raw: checksum error\n");
        return WATER_SENSOR_ERR_I2C;
    }

    out->value = (buf[0] << 8) | buf[1];
    out->min = (buf[2] << 8) | buf[3];
    out->max = (buf[4] << 8) | buf[5];
    out->valid = buf[6] != 0;

    return WATER_SENSOR_OK;
}

int water_sensor_read_temperature_raw(const water_sensor_t *dev, uint8_t channel, water_sensor_temperature_raw_t *out)
{
    assert(out != NULL);

    uint8_t buf[WATER_SENSOR_TEMPERATURE_RAW_SIZE + 1];
    uint16_t reg = (WATER_SENSOR_READ_TEMPERATURE_RAW << 8) | channel;

    if (_read_reg16(dev, reg, buf, sizeof(buf)) != 0) {
        DEBUG("[water_sensor] water_sensor_read_temperature_raw: failed\n");
        return WATER_SENSOR_ERR_I2C;
    }

    if (_checksum(buf, WATER_SENSOR_TEMPERATURE_RAW_SIZE) != buf[7]) {
        DEBUG("[water_sensor] water_sensor_read_temperature_raw: checksum error\n");
        return WATER_SENSOR_ERR_I2C;
    }

    out->value = (buf[0] << 8) | buf[1];
    out->min = (buf[2] << 8) | buf[3];
    out->max = (buf[4] << 8) | buf[5];
    out->valid = buf[6] != 0;

    return WATER_SENSOR_OK;
}

int water_sensor_read_config(const water_sensor_t *dev, water_sensor_config_t *out)
{
    assert(out != NULL);

    uint8_t buf[WATER_SENSOR_CONFIG_SIZE + 1];

    if (_read_reg(dev, WATER_SENSOR_READ_CONFIG, buf, sizeof(buf)) != 0) {
        DEBUG("[water_sensor] water_sensor_read_config: failed\n");
        return WATER_SENSOR_ERR_I2C;
    }

    if (_checksum(buf, WATER_SENSOR_CONFIG_SIZE) != buf[2]) {
        DEBUG("[water_sensor] water_sensor_read_config: checksum error\n");
        return WATER_SENSOR_ERR_I2C;
    }

    out->default_level = (buf[0] << 8) | buf[1];

    return WATER_SENSOR_OK;
}

int water_sensor_write_config(const water_sensor_t *dev, const water_sensor_config_t *in)
{
    assert(in != NULL);

    uint8_t buf[WATER_SENSOR_CONFIG_SIZE + 1];

    buf[0] = (in->default_level & 0xff00) >> 8;
    buf[1] = (in->default_level & 0x00ff) >> 0;
    buf[2] = _checksum(buf, WATER_SENSOR_CONFIG_SIZE);

    if (_write_reg(dev, WATER_SENSOR_WRITE_CONFIG, buf, sizeof(buf)) != 0) {
        DEBUG("[water_sensor] water_sensor_write_config: failed\n");
        return WATER_SENSOR_ERR_I2C;
    }

    return WATER_SENSOR_OK;
}

int water_sensor_read_level_config(const water_sensor_t *dev, uint8_t channel, water_sensor_level_config_t *out)
{
    assert(out != NULL);

    uint8_t buf[WATER_SENSOR_LEVEL_CONFIG_SIZE + 1];
    uint16_t reg = (WATER_SENSOR_READ_LEVEL_CONFIG << 8) | channel;

    if (_read_reg16(dev, reg, buf, sizeof(buf)) != 0) {
        DEBUG("[water_sensor] water_sensor_read_level_config: failed\n");
        return WATER_SENSOR_ERR_I2C;
    }

    if (_checksum(buf, WATER_SENSOR_LEVEL_CONFIG_SIZE) != buf[8]) {
        DEBUG("[water_sensor] water_sensor_read_level_config: checksum error\n");
        return WATER_SENSOR_ERR_I2C;
    }

    out->enabled = buf[0] != 0;
    out->samples = (buf[1] << 8) | buf[2];
    out->alpha = buf[3];
    out->offset = (buf[4] << 8) | buf[5];
    out->level = (buf[6] << 8) | buf[7];

    return WATER_SENSOR_OK;
}

int water_sensor_write_level_config(const water_sensor_t *dev, uint8_t channel, const water_sensor_level_config_t *in)
{
    assert(in != NULL);

    uint8_t buf[WATER_SENSOR_LEVEL_CONFIG_SIZE + 1];
    uint16_t reg = (WATER_SENSOR_WRITE_LEVEL_CONFIG << 8) | channel;

    buf[0] = in->enabled ? 1 : 0;
    buf[1] = (in->samples & 0xff00) >> 8;
    buf[2] = (in->samples & 0x00ff) >> 0;
    buf[3] = in->alpha;
    buf[4] = (in->offset & 0xff00) >> 8;
    buf[5] = (in->offset & 0x00ff) >> 0;
    buf[6] = (in->level & 0xff00) >> 8;
    buf[7] = (in->level & 0x00ff) >> 0;
    buf[8] = _checksum(buf, WATER_SENSOR_LEVEL_CONFIG_SIZE);

    if (_write_reg16(dev, reg, buf, sizeof(buf)) != 0) {
        DEBUG("[water_sensor] water_sensor_write_level_config: failed\n");
        return WATER_SENSOR_ERR_I2C;
    }

    return WATER_SENSOR_OK;
}

int water_sensor_read_temperature_config(const water_sensor_t *dev, uint8_t channel, water_sensor_temperature_config_t *out)
{
    assert(out != NULL);

    uint8_t buf[WATER_SENSOR_TEMPERATURE_CONFIG_SIZE + 1];
    uint16_t reg = (WATER_SENSOR_READ_TEMPERATURE_CONFIG << 8) | channel;

    if (_read_reg16(dev, reg, buf, sizeof(buf)) != 0) {
        DEBUG("[water_sensor] water_sensor_read_temperature_config: failed\n");
        return WATER_SENSOR_ERR_I2C;
    }

    if (_checksum(buf, WATER_SENSOR_TEMPERATURE_CONFIG_SIZE) != buf[4]) {
        DEBUG("[water_sensor] water_sensor_read_temperature_config: checksum error\n");
        return WATER_SENSOR_ERR_I2C;
    }

    out->enabled = buf[0] != 0;
    out->alpha = buf[1];
    out->reference = (buf[2] << 8) | buf[3];

    return WATER_SENSOR_OK;
}

int water_sensor_write_temperature_config(const water_sensor_t *dev, uint8_t channel, const water_sensor_temperature_config_t *in)
{
    assert(in != NULL);

    uint8_t buf[WATER_SENSOR_TEMPERATURE_CONFIG_SIZE + 1];
    uint16_t reg = (WATER_SENSOR_WRITE_TEMPERATURE_CONFIG << 8) | channel;

    buf[0] = in->enabled ? 1 : 0;
    buf[1] = in->alpha;
    buf[2] = (in->reference & 0xff00) >> 8;
    buf[3] = (in->reference & 0x00ff) >> 0;
    buf[4] = _checksum(buf, WATER_SENSOR_TEMPERATURE_CONFIG_SIZE);

    if (_write_reg16(dev, reg, buf, sizeof(buf)) != 0) {
        DEBUG("[water_sensor] water_sensor_write_temperature_config: failed\n");
        return WATER_SENSOR_ERR_I2C;
    }

    return WATER_SENSOR_OK;
}