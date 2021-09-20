#pragma once

#include <stdint.h>

#define NUM_SENSORS 8U
#define NUM_CHANNELS 4U

#define PIN_CONFIG_0 5
#define PIN_CONFIG_1 6
#define PIN_CONFIG_2 7
#define PIN_CONFIG_3 10

#define PIN_MASTER_SDA 8
#define PIN_MASTER_SCL 9

#define PIN_TEMPERATURE 6

#define CONFIG_MAGIC 0xbaab1234

typedef struct {
    uint8_t id;
    uint8_t index;
    uint8_t children;
} info_t;

typedef struct {
    struct {
        bool enabled;
        uint8_t samples;
        uint8_t alpha;
        uint16_t offset;
        int16_t level;
    } adc[NUM_CHANNELS];

    struct {
        uint8_t alpha;
        uint16_t reference;
        bool enabled;
    } temperature;
} config_sensor_t;

typedef struct {
    uint32_t magic;

    int16_t defaultLevel;

    config_sensor_t sensors[NUM_SENSORS];
} config_t;

typedef struct {
    struct {
        uint16_t value;
        uint16_t min;
        uint16_t max;
        bool valid;
    } adc[NUM_CHANNELS];

    struct {
        int16_t value;
        int16_t min;
        int16_t max;
        bool valid;
    } temperature;
} state_sensor_t;

typedef struct {
    bool enabled;

    uint8_t errors;
    uint8_t context;

    struct {
        int16_t value;
        int8_t channel;
        bool valid;
    } level;

    struct {
        int16_t value;
        int8_t channel;
        bool valid;
    } temperature;

    state_sensor_t sensors[NUM_SENSORS];
} state_t;

void reset();
void init();
void enable();
void store();
void load();
void calibrate();
void zero();
