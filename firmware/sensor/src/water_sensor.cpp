#include "water_sensor.h"

#include "assert.h"

static uint8_t _checksum(const uint8_t *data, size_t length)
{
    uint8_t checksum = 0xff;

    for (unsigned i = 0; i < length; i++) {
        checksum ^= data[i];
    }

    return checksum;
}

WaterSensor::WaterSensor()
{
}

int WaterSensor::init()
{
    /* reset the device */
    if (WaterSensor::reset() != WATER_SENSOR_OK) {
        return WATER_SENSOR_ERR_I2C;
    }

    /* read sensor identification */
    water_sensor_info_t info;

    if (WaterSensor::readInfo(&info) != WATER_SENSOR_OK) {
        return WATER_SENSOR_ERR_I2C;
    }

    if (info.id != WATER_SENSOR_ID) {
        return WATER_SENSOR_ERR_I2C;
    }

    if (info.errors != 0) {
        return WATER_SENSOR_ERR_I2C;
    }

    return WATER_SENSOR_OK;
}

int WaterSensor::reset()
{
    if (cmd(WATER_SENSOR_RESET) != 0) {
        return WATER_SENSOR_ERR_I2C;
    }

    return WATER_SENSOR_OK;
}

int WaterSensor::enable()
{
    if (cmd(WATER_SENSOR_ENABLE) != 0) {
        return WATER_SENSOR_ERR_I2C;
    }

    return WATER_SENSOR_OK;
}

int WaterSensor::load()
{
    if (cmd(WATER_SENSOR_LOAD) != 0) {
        return WATER_SENSOR_ERR_I2C;
    }

    return WATER_SENSOR_OK;
}

int WaterSensor::store()
{
    if (cmd(WATER_SENSOR_STORE) != 0) {
        return WATER_SENSOR_ERR_I2C;
    }

    return WATER_SENSOR_OK;
}

int WaterSensor::calibrate()
{
    if (cmd(WATER_SENSOR_CALIBRATE) != 0) {
        return WATER_SENSOR_ERR_I2C;
    }

    return WATER_SENSOR_OK;
}

int WaterSensor::zero()
{
    if (cmd(WATER_SENSOR_ZERO) != 0) {
        return WATER_SENSOR_ERR_I2C;
    }

    return WATER_SENSOR_OK;
}

int WaterSensor::readInfo(water_sensor_info_t *out)
{
    assert(out != NULL);

    uint8_t buf[WATER_SENSOR_INFO_SIZE + 1];

    if (read_reg(WATER_SENSOR_READ_INFO, buf, sizeof(buf)) != 0) {
        return WATER_SENSOR_ERR_I2C;
    }

    if (_checksum(buf, WATER_SENSOR_INFO_SIZE) != buf[6]) {
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

int WaterSensor::readLevel(water_sensor_level_t *out)
{
    assert(out != NULL);

    uint8_t buf[WATER_SENSOR_LEVEL_SIZE + 1];

    if (read_reg(WATER_SENSOR_READ_LEVEL, buf, sizeof(buf)) != 0) {
        return WATER_SENSOR_ERR_I2C;
    }

    if (_checksum(buf, WATER_SENSOR_TEMPERATURE_SIZE) != buf[4]) {
        return WATER_SENSOR_ERR_I2C;
    }

    out->value = (buf[0] << 8) | buf[1];
    out->channel = buf[2];
    out->valid = buf[3] != 0;

    return WATER_SENSOR_OK;
}

int WaterSensor::readTemperature(water_sensor_temperature_t *out)
{
    assert(out != NULL);

    uint8_t buf[WATER_SENSOR_TEMPERATURE_SIZE + 1];

    if (read_reg(WATER_SENSOR_READ_TEMPERATURE, buf, sizeof(buf)) != 0) {
        return WATER_SENSOR_ERR_I2C;
    }

    if (_checksum(buf, WATER_SENSOR_TEMPERATURE_SIZE) != buf[4]) {
        return WATER_SENSOR_ERR_I2C;
    }

    out->value = (buf[0] << 8) | buf[1];
    out->channel = buf[2];
    out->valid = buf[3] != 0;

    return WATER_SENSOR_OK;
}

int WaterSensor::readLevelRaw(uint8_t channel, water_sensor_level_raw_t *out)
{
    assert(out != NULL);

    uint8_t buf[WATER_SENSOR_LEVEL_RAW_SIZE + 1];
    uint16_t reg = (WATER_SENSOR_READ_LEVEL_RAW << 8) | channel;

    if (read_reg16(reg, buf, sizeof(buf)) != 0) {
        return WATER_SENSOR_ERR_I2C;
    }

    if (_checksum(buf, WATER_SENSOR_LEVEL_RAW_SIZE) != buf[7]) {
        return WATER_SENSOR_ERR_I2C;
    }

    out->value = (buf[0] << 8) | buf[1];
    out->min = (buf[2] << 8) | buf[3];
    out->max = (buf[4] << 8) | buf[5];
    out->valid = buf[6] != 0;

    return WATER_SENSOR_OK;
}

int WaterSensor::readTemperatureRaw(uint8_t channel, water_sensor_temperature_raw_t *out)
{
    assert(out != NULL);

    uint8_t buf[WATER_SENSOR_TEMPERATURE_RAW_SIZE + 1];
    uint16_t reg = (WATER_SENSOR_READ_TEMPERATURE_RAW << 8) | channel;

    if (read_reg16(reg, buf, sizeof(buf)) != 0) {
        return WATER_SENSOR_ERR_I2C;
    }

    if (_checksum(buf, WATER_SENSOR_TEMPERATURE_RAW_SIZE) != buf[7]) {
        return WATER_SENSOR_ERR_I2C;
    }

    out->value = (buf[0] << 8) | buf[1];
    out->min = (buf[2] << 8) | buf[3];
    out->max = (buf[4] << 8) | buf[5];
    out->valid = buf[6] != 0;

    return WATER_SENSOR_OK;
}

int WaterSensor::readConfig(water_sensor_config_t *out)
{
    assert(out != NULL);

    uint8_t buf[WATER_SENSOR_CONFIG_SIZE + 1];

    if (read_reg(WATER_SENSOR_READ_CONFIG, buf, sizeof(buf)) != 0) {
        return WATER_SENSOR_ERR_I2C;
    }

    if (_checksum(buf, WATER_SENSOR_CONFIG_SIZE) != buf[2]) {
        return WATER_SENSOR_ERR_I2C;
    }

    out->default_level = (buf[0] << 8) | buf[1];

    return WATER_SENSOR_OK;
}

int WaterSensor::writeConfig(const water_sensor_config_t *in)
{
    assert(in != NULL);

    uint8_t buf[WATER_SENSOR_CONFIG_SIZE + 1];

    buf[0] = (in->default_level & 0xff00) >> 8;
    buf[1] = (in->default_level & 0x00ff) >> 0;
    buf[2] = _checksum(buf, WATER_SENSOR_CONFIG_SIZE);

    if (write_reg(WATER_SENSOR_WRITE_CONFIG, buf, sizeof(buf)) != 0) {
        return WATER_SENSOR_ERR_I2C;
    }

    return WATER_SENSOR_OK;
}

int WaterSensor::readLevelConfig(uint8_t channel, water_sensor_level_config_t *out)
{
    assert(out != NULL);

    uint8_t buf[WATER_SENSOR_LEVEL_CONFIG_SIZE + 1];
    uint16_t reg = (WATER_SENSOR_READ_LEVEL_CONFIG << 8) | channel;

    if (read_reg16(reg, buf, sizeof(buf)) != 0) {
        return WATER_SENSOR_ERR_I2C;
    }

    if (_checksum(buf, WATER_SENSOR_LEVEL_CONFIG_SIZE) != buf[8]) {
        return WATER_SENSOR_ERR_I2C;
    }

    out->enabled = buf[0] != 0;
    out->samples = (buf[1] << 8) | buf[2];
    out->alpha = buf[3];
    out->offset = (buf[4] << 8) | buf[5];
    out->level = (buf[6] << 8) | buf[7];

    return WATER_SENSOR_OK;
}

int WaterSensor::writeLevelConfig(uint8_t channel, const water_sensor_level_config_t *in)
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

    if (write_reg16(reg, buf, sizeof(buf)) != 0) {
        return WATER_SENSOR_ERR_I2C;
    }

    return WATER_SENSOR_OK;
}

int WaterSensor::readTemperatureConfig(uint8_t channel, water_sensor_temperature_config_t *out)
{
    assert(out != NULL);

    uint8_t buf[WATER_SENSOR_TEMPERATURE_CONFIG_SIZE + 1];
    uint16_t reg = (WATER_SENSOR_READ_TEMPERATURE_CONFIG << 8) | channel;

    if (read_reg16(reg, buf, sizeof(buf)) != 0) {
        return WATER_SENSOR_ERR_I2C;
    }

    if (_checksum(buf, WATER_SENSOR_TEMPERATURE_CONFIG_SIZE) != buf[4]) {
        return WATER_SENSOR_ERR_I2C;
    }

    out->enabled = buf[0] != 0;
    out->alpha = buf[1];
    out->reference = (buf[2] << 8) | buf[3];

    return WATER_SENSOR_OK;
}

int WaterSensor::writeTemperatureConfig(uint8_t channel, const water_sensor_temperature_config_t *in)
{
    assert(in != NULL);

    uint8_t buf[WATER_SENSOR_TEMPERATURE_CONFIG_SIZE + 1];
    uint16_t reg = (WATER_SENSOR_WRITE_TEMPERATURE_CONFIG << 8) | channel;

    buf[0] = in->enabled ? 1 : 0;
    buf[1] = in->alpha;
    buf[2] = (in->reference & 0xff00) >> 8;
    buf[3] = (in->reference & 0x00ff) >> 0;
    buf[4] = _checksum(buf, WATER_SENSOR_TEMPERATURE_CONFIG_SIZE);

    if (write_reg16(reg, buf, sizeof(buf)) != 0) {
        return WATER_SENSOR_ERR_I2C;
    }

    return WATER_SENSOR_OK;
}

int WaterSensor::cmd(uint8_t cmd)
{
    _wire->beginTransmission(_address);
    _wire->write(cmd);
    return _wire->endTransmission();
}

int WaterSensor::read_reg(uint8_t reg, uint8_t *data, size_t length)
{
    int result;

    _wire->beginTransmission(_address);
    _wire->write(uint8_t(reg));
    result = _wire->endTransmission();

    if (result != 0) {
        return result;
    }

    result = _wire->requestFrom(_address, length);

    if (result != length) {
        return -1;
    }

    for (unsigned i = 0; i < length; i++) {
        data[i] = _wire->read();
    }

    return 0;
}

int WaterSensor::read_reg16(uint16_t reg, uint8_t *data, size_t length)
{
    int result;

    _wire->beginTransmission(_address);
    _wire->write(uint8_t((reg & 0xff00) >> 8));
    _wire->write(uint8_t((reg & 0x00ff) >> 0));
    result = _wire->endTransmission();

    if (result != 0) {
        return result;
    }

    result = _wire->requestFrom(_address, length);

    if (result != length) {
        return -1;
    }

    for (unsigned i = 0; i < length; i++) {
        data[i] = _wire->read();
    }

    return 0;
}

int WaterSensor::write_reg(uint8_t reg, const uint8_t *data, size_t length)
{
    _wire->beginTransmission(_address);
    _wire->write(uint8_t(reg));

    for (unsigned i = 0; i < length; i++) {
        _wire->write(data[i]);
    }

    return _wire->endTransmission();
}

int WaterSensor::write_reg16(uint16_t reg, const uint8_t *data, size_t length)
{
    _wire->beginTransmission(_address);
    _wire->write(uint8_t((reg & 0xff00) >> 8));
    _wire->write(uint8_t((reg & 0x00ff) >> 0));

    for (unsigned i = 0; i < length; i++) {
        _wire->write(data[i]);
    }

    return _wire->endTransmission();
}
