#include <stdint.h>

#include <ADCTouch.h>
#include <Arduino.h>
#include <EEPROM.h>
#include <HardWire.h>
#include <SoftWire.h>

#include "main.h"
#include "water_sensor.h"

static info_t info;
static config_t config;
static state_t state;

static WaterSensor waterSensor[NUM_SENSORS];

static SoftWire Wire2(PIN_MASTER_SDA, PIN_MASTER_SCL);

static char swTxBuffer[32];
static char swRxBuffer[32];

static AsyncDelay readTimer;
static AsyncDelay updateTimer;

// I2C response structure.
static struct {
    uint8_t buffer[32];
    size_t length;
    bool nack;
} response;

static bool _read(size_t length)
{
    uint8_t checksum = 0xff;

    for (unsigned i = 0; i < length; i++) {
        response.buffer[i] = Wire.read();
        checksum ^= response.buffer[i];
    }

    return checksum == Wire.read();
}

void receiveEvent(int countToRead)
{
    // Reset response, so that no stale date is read.
    response.length = 0;
    response.nack = false;

    // Read the command.
    uint8_t data = Wire.read();

    // Serial.print("Parent data: ");
    // Serial.println(data, HEX);

    switch (data) {
        case WATER_SENSOR_RESET:
        {
            reset();
            break;
        }
        case WATER_SENSOR_ENABLE:
        {
            enable();
            break;
        }
        case WATER_SENSOR_LOAD:
        {
            load();
            break;
        }
        case WATER_SENSOR_STORE:
        {
            store();
            break;
        }
        case WATER_SENSOR_CALIBRATE:
        {
            calibrate();
            break;
        }
        case WATER_SENSOR_ZERO:
        {
            zero();
            break;
        }
        case WATER_SENSOR_READ_INFO:
        {
            int sensors = 1 + info.children;

            response.buffer[0] = info.id;
            response.buffer[1] = sensors * NUM_CHANNELS;
            response.buffer[2] = sensors;
            response.buffer[3] = state.enabled ? 1 : 0;
            response.buffer[4] = state.errors;
            response.buffer[5] = state.context;

            response.length = WATER_SENSOR_INFO_SIZE;

            break;
        }
        case WATER_SENSOR_READ_LEVEL:
        {
            response.buffer[0] = (state.level.value & 0xff00) >> 8;
            response.buffer[1] = (state.level.value & 0x00ff) >> 0;
            response.buffer[2] = state.level.channel;
            response.buffer[3] = state.level.valid;

            response.length = WATER_SENSOR_LEVEL_SIZE;

            break;
        }
        case WATER_SENSOR_READ_TEMPERATURE:
        {
            response.buffer[0] = (state.temperature.value & 0xff00) >> 8;
            response.buffer[1] = (state.temperature.value & 0x00ff) >> 0;
            response.buffer[2] = state.temperature.channel;
            response.buffer[3] = state.temperature.valid;

            response.length = WATER_SENSOR_TEMPERATURE_SIZE;

            break;
        }
        case WATER_SENSOR_READ_LEVEL_RAW:
        {
            if (countToRead != 2) {
                response.nack = true;
                return;
            }

            int channel = Wire.read();

            int i = channel / NUM_CHANNELS;
            int j = channel % NUM_CHANNELS;

            response.buffer[0] = (state.sensors[i].adc[j].value & 0xff00) >> 8;
            response.buffer[1] = (state.sensors[i].adc[j].value & 0x00ff) >> 0;
            response.buffer[2] = (state.sensors[i].adc[j].min & 0xff00) >> 8;
            response.buffer[3] = (state.sensors[i].adc[j].min & 0x00ff) >> 0;
            response.buffer[4] = (state.sensors[i].adc[j].max & 0xff00) >> 8;
            response.buffer[5] = (state.sensors[i].adc[j].max & 0x00ff) >> 0;
            response.buffer[6] = state.sensors[i].adc[j].valid;

            response.length = WATER_SENSOR_LEVEL_RAW_SIZE;

            break;
        }
        case WATER_SENSOR_READ_TEMPERATURE_RAW:
        {
            if (countToRead != 2) {
                response.nack = true;
                return;
            }

            int channel = Wire.read();

            int i = channel % NUM_SENSORS;

            response.buffer[0] = (state.sensors[i].temperature.value & 0xff00) >> 8;
            response.buffer[1] = (state.sensors[i].temperature.value & 0x00ff) >> 0;
            response.buffer[2] = (state.sensors[i].temperature.min & 0xff00) >> 8;
            response.buffer[3] = (state.sensors[i].temperature.min & 0x00ff) >> 0;
            response.buffer[4] = (state.sensors[i].temperature.max & 0xff00) >> 8;
            response.buffer[5] = (state.sensors[i].temperature.max & 0x00ff) >> 0;
            response.buffer[6] = state.sensors[i].temperature.valid;

            response.length = WATER_SENSOR_TEMPERATURE_RAW_SIZE;

            break;
        }
        case WATER_SENSOR_READ_CONFIG:
        {
            response.buffer[0] = (config.defaultLevel & 0xff00) >> 8;
            response.buffer[1] = (config.defaultLevel & 0x00ff) >> 0;

            response.length = WATER_SENSOR_CONFIG_SIZE;

            break;
        }
        case WATER_SENSOR_WRITE_CONFIG:
        {
            if (countToRead != 4) {
                response.nack = true;
                return;
            }

            if (!_read(WATER_SENSOR_CONFIG_SIZE)) {
                response.nack = true;
                return;
            }

            config.defaultLevel = (response.buffer[0] << 8) | response.buffer[1];

            break;
        }
        case WATER_SENSOR_READ_LEVEL_CONFIG:
        {
            if (countToRead != 2) {
                response.nack = true;
                return;
            }

            int channel = Wire.read();

            int i = channel / NUM_CHANNELS;
            int j = channel % NUM_CHANNELS;

            response.buffer[0] = config.sensors[i].adc[j].enabled ? 1 : 0;
            response.buffer[1] = (config.sensors[i].adc[j].samples & 0xff00) >> 8;
            response.buffer[2] = (config.sensors[i].adc[j].samples & 0x00ff) >> 0;
            response.buffer[3] = config.sensors[i].adc[j].alpha;
            response.buffer[4] = (config.sensors[i].adc[j].offset & 0xff00) >> 8;
            response.buffer[5] = (config.sensors[i].adc[j].offset & 0x00ff) >> 0;
            response.buffer[6] = (config.sensors[i].adc[j].level & 0xff00) >> 8;
            response.buffer[7] = (config.sensors[i].adc[j].level & 0x00ff) >> 0;

            response.length = WATER_SENSOR_LEVEL_CONFIG_SIZE;

            break;
        }
        case WATER_SENSOR_WRITE_LEVEL_CONFIG:
        {
            if (countToRead != 11) {
                response.nack = true;
                return;
            }

            int channel = Wire.read();

            int i = channel / NUM_CHANNELS;
            int j = channel % NUM_CHANNELS;

            if (!_read(WATER_SENSOR_LEVEL_CONFIG_SIZE)) {
                response.nack = true;
                return;
            }

            config.sensors[i].adc[j].enabled = response.buffer[0] != 0;
            config.sensors[i].adc[j].samples = (response.buffer[1] << 8) | response.buffer[2];
            config.sensors[i].adc[j].alpha = response.buffer[3];
            config.sensors[i].adc[j].offset = (response.buffer[4] << 8) | response.buffer[5];
            config.sensors[i].adc[j].level = (response.buffer[6] << 8) | response.buffer[7];

            break;
        }
        case WATER_SENSOR_READ_TEMPERATURE_CONFIG:
        {
            if (countToRead != 2) {
                response.nack = true;
                return;
            }

            int channel = Wire.read();

            int i = channel % NUM_CHANNELS;

            response.buffer[0] = config.sensors[i].temperature.enabled ? 1 : 0;
            response.buffer[1] = config.sensors[i].temperature.alpha;
            response.buffer[2] = (config.sensors[i].temperature.reference & 0xff00) >> 8;
            response.buffer[3] = (config.sensors[i].temperature.reference & 0x00ff) >> 0;

            response.length = WATER_SENSOR_TEMPERATURE_CONFIG_SIZE;

            break;
        }
        case WATER_SENSOR_WRITE_TEMPERATURE_CONFIG:
        {
            if (countToRead != 7) {
                response.nack = true;
                return;
            }

            int channel = Wire.read();

            int i = channel % NUM_SENSORS;

            if (!_read(WATER_SENSOR_TEMPERATURE_CONFIG_SIZE)) {
                response.nack = true;
                return;
            }

            config.sensors[i].temperature.enabled = response.buffer[0] != 0;
            config.sensors[i].temperature.alpha = response.buffer[1];
            config.sensors[i].temperature.reference = (response.buffer[2] << 8) | response.buffer[3];

            break;
        }
    }
}

void requestEvent()
{
    uint8_t checksum = 0xff;

    if (response.length) {
        for (unsigned i = 0; i < response.length; i++) {
            checksum ^= response.buffer[i];
        }

        Wire.write(response.buffer, response.length);
        Wire.write(checksum);
    }
}

uint8_t readConfigPins(void)
{
    pinMode(PIN_CONFIG_0, INPUT_PULLUP);
    pinMode(PIN_CONFIG_1, INPUT_PULLUP);
    pinMode(PIN_CONFIG_2, INPUT_PULLUP);
    pinMode(PIN_CONFIG_3, INPUT_PULLUP);

    return
        (digitalRead(PIN_CONFIG_0) == HIGH ? 0 : 1) << 0 |
        (digitalRead(PIN_CONFIG_1) == HIGH ? 0 : 1) << 1 |
        (digitalRead(PIN_CONFIG_2) == HIGH ? 0 : 1) << 2 |
        (digitalRead(PIN_CONFIG_3) == HIGH ? 0 : 1) << 3;
}

void setupParent()
{
    Wire.onRequest(requestEvent);
    Wire.onReceive(receiveEvent);
    Wire.begin(0x70);

    Wire2.setTxBuffer(swTxBuffer, sizeof(swTxBuffer));
    Wire2.setRxBuffer(swRxBuffer, sizeof(swRxBuffer));
    Wire2.setDelay_us(5);
    Wire2.setTimeout(500);
    Wire2.begin();

    for (unsigned i = 0; i < NUM_SENSORS; i++) {
        waterSensor[i].setAddress(0x40 + ((i + 1) << 2));
        waterSensor[i].setWire(&Wire2);
    }
}

void setupChild()
{
    Wire.onRequest(requestEvent);
    Wire.onReceive(receiveEvent);
    Wire.begin(0x40 + (info.index << 2));
}

int initChildren()
{
    int result = 0;
    water_sensor_info_t response;

    for (unsigned i = 0; i < info.children; i++) {
        if (waterSensor[i].readInfo(&response) != WATER_SENSOR_OK) {
            result = i + 1;
            continue;
        }

        if (response.id != WATER_SENSOR_ID) {
            result = i + 1;
            continue;
        }
    }

    return result;
}

int resetChildren()
{
    int result = 0;

    for (unsigned i = 0; i < info.children; i++) {
        if (waterSensor[i].reset() != WATER_SENSOR_OK) {
            result = i + 1;
            continue;
        }
    }

    return result;
}

int enableChildren()
{
    int result = 0;

    for (unsigned i = 0; i < info.children; i++) {
        for (unsigned j = 0; j < NUM_CHANNELS; j++) {
            water_sensor_level_config_t request;

            request.enabled = config.sensors[i + 1].adc[j].enabled ? 1 : 0;
            request.samples = config.sensors[i + 1].adc[j].samples;
            request.alpha = config.sensors[i + 1].adc[j].alpha;
            request.offset = config.sensors[i + 1].adc[j].offset;
            request.level = config.sensors[i + 1].adc[j].level;

            if (waterSensor[i].writeLevelConfig(j, &request) != WATER_SENSOR_OK) {
                result = i + 1;
                continue;
            }
        }

        water_sensor_temperature_config_t request;

        request.enabled = config.sensors[i + 1].temperature.enabled ? 1 : 0;
        request.alpha = config.sensors[i + 1].temperature.alpha;
        request.reference = config.sensors[i + 1].temperature.reference;

        if (waterSensor[i].writeTemperatureConfig(0, &request) != WATER_SENSOR_OK) {
            result = i + 1;
            continue;
        }
    }

    return result;
}

int readChildren()
{
    int result = 0;

    for (unsigned i = 0; i < info.children; i++) {
        // Read raw level.
        for (unsigned j = 0; j < NUM_CHANNELS; j++) {
            water_sensor_level_raw_t response;

            if (waterSensor[i].readLevelRaw(j, &response) != WATER_SENSOR_OK) {
                result = 1 + i;
                continue;
            }

            state.sensors[1 + i].adc[j].value = response.value;
            state.sensors[1 + i].adc[j].min = response.min;
            state.sensors[1 + i].adc[j].max = response.max;
            state.sensors[1 + i].adc[j].valid = response.valid;
        }

        // Read raw temperature.
        water_sensor_temperature_raw_t response;

        if (waterSensor[i].readTemperatureRaw(0, &response) != WATER_SENSOR_OK) {
            result = 1 + i;
            continue;
        }

        state.sensors[1 + i].temperature.value = response.value;
        state.sensors[1 + i].temperature.min = response.min;
        state.sensors[1 + i].temperature.max = response.max;
        state.sensors[1 + i].temperature.valid = response.valid;
    }

    return result;
}

int zeroChildren()
{
    int result = 0;

    for (unsigned i = 0; i < info.children; i++) {
        if (waterSensor[i].zero() != WATER_SENSOR_OK) {
            result = 1 + i;
            continue;
        }
    }

    return result;
}

void reset()
{
    int result;

    state.enabled = false;
    state.errors = 0;
    state.context = 0;

    for (unsigned i = 0; i < NUM_SENSORS; i++) {
        for (unsigned j = 0; j < NUM_CHANNELS; j++) {
            config.sensors[i].adc[j].enabled = true;
            config.sensors[i].adc[j].samples = 60;
            config.sensors[i].adc[j].alpha = 25;
            config.sensors[i].adc[j].offset = 512;
            config.sensors[i].adc[j].level = (NUM_SENSORS * NUM_CHANNELS) - (i * NUM_SENSORS) - j;

            state.sensors[i].adc[j].value = 0;
            state.sensors[i].adc[j].min = UINT16_MAX;
            state.sensors[i].adc[j].max = 0;
            state.sensors[i].adc[j].valid = false;
        }

        config.sensors[i].temperature.enabled = true;
        config.sensors[i].temperature.alpha = 25;
        config.sensors[i].temperature.reference = 5000;

        state.sensors[i].temperature.value = 0;
        state.sensors[i].temperature.min = INT16_MAX;
        state.sensors[i].temperature.max = INT16_MIN;
        state.sensors[i].temperature.valid = false;
    }

    if (info.index == 0) {
        // Detect children.
        result = initChildren();

        if (result != 0) {
            state.errors |= 1 << WATER_SENSOR_INFO_ERRORS_INIT;
            state.context = result;
            return;
        }

        // Reset children.
        result = resetChildren();

        if (result != 0) {
            state.errors |= 1 << WATER_SENSOR_INFO_ERRORS_RESET;
            state.context = result;
            return;
        }
    }
}

void enable()
{
    int result;

    if (info.index == 0) {
        // Enable children.
        result = enableChildren();

        if (result != 0) {
            state.errors |= 1 << WATER_SENSOR_INFO_ERRORS_ENABLE;
            state.context = result;
            return;
        }
    }

    state.enabled = true;
}

void load()
{
    noInterrupts();

    config_t buffer;
    uint8_t checksum = 0xff;

    for (unsigned i = 0; i < sizeof(config_t); i++) {
        uint8_t result = EEPROM.read(i);

        ((uint8_t *)&buffer)[i] = result;
        checksum ^= result;
    }

    if (buffer.magic != CONFIG_MAGIC) {
        Serial.println("Magic failed.");
        return;
    }

    if (checksum != EEPROM.read(sizeof(config_t))) {
        Serial.println("Checksum failed.");
        return;
    }

    memcpy(&config, &buffer, sizeof(config_t));

    interrupts();
}

void store()
{
    noInterrupts();

    config.magic = CONFIG_MAGIC;

    uint8_t checksum = 0xff;

    for (unsigned i = 0; i < sizeof(config_t); i++) {
        uint8_t result = ((uint8_t *)&config)[i];

        EEPROM.write(i, result);
        checksum ^= result;
    }

    EEPROM.write(sizeof(config_t), checksum);

    interrupts();
}

void calibrate()
{
    uint16_t min = 0;
    uint16_t max = 0;

    for (unsigned i = 0; i < (1U + info.children); i++) {
        for (unsigned j = 0; j < NUM_CHANNELS; j++) {
            if (!config.sensors[i].adc[j].enabled) {
                continue;
            }

            min = state.sensors[i].adc[j].min;
            max = state.sensors[i].adc[j].max;
        }
    }

    if (min > max) {
        return;
    }

    uint16_t offset = min + ((max - min) / 3);

    for (unsigned i = 0; i < (1U + info.children); i++) {
        for (unsigned j = 0; j < NUM_CHANNELS; j++) {
            config.sensors[i].adc[j].offset = offset;
        }
    }
}

void zero()
{
    int result;

    for (unsigned i = 0; i < NUM_SENSORS; i++) {
        for (unsigned j = 0; j < NUM_CHANNELS; j++) {
            state.sensors[i].adc[j].min = UINT16_MAX;
            state.sensors[i].adc[j].max = 0;
            state.sensors[i].adc[j].valid = false;
        }

        state.sensors[i].temperature.min = INT16_MAX;
        state.sensors[i].temperature.max = INT16_MIN;
    }

    if (info.index == 0) {
        result = zeroChildren();

        if (result != 0) {
            state.errors |= WATER_SENSOR_INFO_ERRORS_ZERO;
            state.context = result;
        }
    }
}

void setup()
{
    Serial.begin(9600);

    // Read the configuration pins.
    uint8_t pins = readConfigPins();

    info.id = WATER_SENSOR_ID;

    if (pins & 0x08) {
        info.index = 0;
        info.children = pins & 0x07;
    }
    else {
        info.index = (pins & 0x07) + 1;
        info.children = 0;
    }

    Serial.print("Pins: index = ");
    Serial.print(info.index, DEC);
    Serial.print(", children = ");
    Serial.println(info.children, DEC);

    // Configure sensor as parent or child.
    if (info.index == 0) {
        setupParent();

        // Add a delay, because when powering all sensors at the same time,
        // it is likely that the child sensors are still powering up.
        delay(250);
    }
    else {
        setupChild();
    }

    // Setup timers
    readTimer.start(250, AsyncDelay::MILLIS);

    if (info.index == 0) {
        updateTimer.start(250, AsyncDelay::MILLIS);
    }

    // Initialize config and state.
    reset();
}

void readLocal()
{
    int pins[NUM_CHANNELS] = { A0, A1, A2, A3 };

    for (unsigned j = 0; j < NUM_CHANNELS; j++) {
        if (config.sensors[0].adc[j].enabled) {
            uint32_t lastValue = state.sensors[0].adc[j].value;
            uint32_t newValue = ADCTouch.read(
                pins[j],
                config.sensors[0].adc[j].samples
            );

            uint8_t alpha = config.sensors[0].adc[j].alpha;

            state.sensors[0].adc[j].value = (uint16_t)(((newValue * alpha) + ((100  - alpha) * lastValue)) / 100 );
            state.sensors[0].adc[j].min = min(state.sensors[0].adc[j].min, state.sensors[0].adc[j].value);
            state.sensors[0].adc[j].max = max(state.sensors[0].adc[j].max, state.sensors[0].adc[j].value);
            state.sensors[0].adc[j].valid = true;
        }
    }

    if (config.sensors[0].temperature.enabled) {
        analogRead(PIN_TEMPERATURE);
        int sensorValue = analogRead(PIN_TEMPERATURE);

        double millivolt = sensorValue * (config.sensors[0].temperature.reference / 1000.0);
        double tempC =  (13.582 - sqrt(pow(-13.5820, 2) + (0.01732) * (2230.8 - millivolt))) / -0.00866 + 30;

        double temperature = tempC * 100.0;

        uint32_t lastValue = state.sensors[0].temperature.value;
        uint32_t newValue = (int32_t)temperature;

        uint8_t alpha = config.sensors[0].temperature.alpha;

        state.sensors[0].temperature.value = (uint16_t)(((newValue * alpha) + ((100  - alpha) * lastValue)) / 100);
        state.sensors[0].temperature.min = min(state.sensors[0].temperature.min, state.sensors[0].temperature.value);
        state.sensors[0].temperature.max = max(state.sensors[0].temperature.max, state.sensors[0].temperature.value);
        state.sensors[0].temperature.valid = true;
    }
}

void updateState()
{
    bool found = false;

    struct {
        int16_t value = 0;
        int8_t channel;
        bool valid = true;
    } level;

    struct {
        int16_t value = 0;
        int16_t lowest;
        int8_t channel;
        int32_t average = 0;
        uint8_t count = 0;
        bool valid = true;
    } temperature;

    // If water is detected by channel X, then it is assumed that the channels
    // X + 1..N detect water as well (their ADC values exceeds their offsets).
    // If this is the case, then the level value configured for channel X is
    // reported. If no channel detects water, then use the default level value
    // stored in configuration.
    for (unsigned i = 0; i < (1U + info.children); i++) {
        for (unsigned j = 0; j < NUM_CHANNELS; j++) {
            uint8_t channel = (i * NUM_CHANNELS) + j;

            if (!config.sensors[i].adc[j].enabled) {
                continue;
            }

            if (state.sensors[i].adc[j].value > config.sensors[i].adc[j].offset) {
                if (!found) {
                    level.value = config.sensors[i].adc[j].level;
                    level.channel = channel;
                    found = true;
                }
            }
            else {
                found = false;
            }

            level.valid |= state.sensors[i].adc[j].valid;
        }
    }

    if (!found) {
        level.value = config.defaultLevel;
        level.channel = -1;
    }

    // For the temperature, take a weighted average of the temperature
    // sensors that have channels that detected water. If N out of M channels
    // of sensor X have water detected, then the temperature value for sensor
    // X is weigthed N times in the average.
    for (unsigned i = 0; i < (1U + info.children); i++) {
        if (!config.sensors[i].temperature.enabled) {
            continue;
        }

        temperature.lowest = state.sensors[i].temperature.value;
        temperature.valid |= state.sensors[i].temperature.valid;
    }

    if (found) {
        for (unsigned i = 0; i < (1U + info.children); i++) {
            for (unsigned j = 0; j < NUM_CHANNELS; j++) {
                uint8_t channel = (i * NUM_CHANNELS) + j;

                if (channel >= level.channel) {
                    temperature.average += state.sensors[i].temperature.value;
                    temperature.count++;
                }
            }
        }

        temperature.value = temperature.average / temperature.count;
        temperature.channel = (((1 + info.children) * NUM_CHANNELS) - level.channel) / NUM_CHANNELS;
    }
    else {
        temperature.value = temperature.lowest;
        temperature.channel = -1;
    }

    // Update the global state.
    state.level.value = level.value;
    state.level.channel = level.channel;
    state.level.valid = level.valid;

    state.temperature.value = temperature.value;
    state.temperature.channel = temperature.channel;
    state.temperature.valid = temperature.valid;
}

void loop()
{
    int result;

    // Update the local sensors.
    if (readTimer.isExpired()) {
        readLocal();

        // Reset timer.
        readTimer.repeat();
    }

    // Update the remote sensors.
    if (info.index == 0) {
        if (updateTimer.isExpired()) {
            if (state.enabled) {
                result = readChildren();

                if (result != 0) {
                    state.errors |= 1 << WATER_SENSOR_INFO_ERRORS_READ;
                    state.context = result;
                }

                updateState();
            }

            // Reset timer.
            updateTimer.repeat();
        }
    }
}