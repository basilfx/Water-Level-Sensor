#pragma once

#include <stdint.h>

#include <SoftWire.h>

#include "water_sensor_internals.h"

#define WATER_SENSOR_OK 0
#define WATER_SENSOR_ERR_I2C -1

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

class WaterSensor {
public:
    WaterSensor();

    inline void setWire(SoftWire* wire)
    {
        _wire = wire;
    }

    inline void setAddress(uint8_t address)
    {
        _address = address;
    }

    int init();
    int reset();
    int enable();
    int load();
    int store();
    int calibrate();
    int zero();

    int readInfo(water_sensor_info_t *out);
    int readLevel(water_sensor_level_t *out);
    int readTemperature(water_sensor_temperature_t *out);
    int readLevelRaw(uint8_t channel, water_sensor_level_raw_t *out);
    int readTemperatureRaw(uint8_t channel, water_sensor_temperature_raw_t *out);
    int readConfig(water_sensor_config_t *out);
    int writeConfig(const water_sensor_config_t *in);
    int readLevelConfig(uint8_t channel, water_sensor_level_config_t *out);
    int writeLevelConfig(uint8_t channel, const water_sensor_level_config_t *in);
    int readTemperatureConfig(uint8_t channel, water_sensor_temperature_config_t *out);
    int writeTemperatureConfig(uint8_t channel, const water_sensor_temperature_config_t *in);

private:
    SoftWire *_wire;

    uint8_t _address;

    int cmd(uint8_t cmd);
    int read_reg(uint8_t reg, uint8_t *data, size_t length);
    int read_reg16(uint16_t reg, uint8_t *data, size_t length);
    int write_reg(uint8_t reg, const uint8_t *data, size_t length);
    int write_reg16(uint16_t reg, const uint8_t *data, size_t length);
};