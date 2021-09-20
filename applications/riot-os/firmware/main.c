/*
 * Copyright (C) 2021 Bas Stottelaar <basstottelaar@gmail.com>
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "water_sensor.h"
#include "water_sensor_internals.h"

#include "board.h"
#include "xtimer.h"
#include "shell.h"

#if IS_ACTIVE(MODULE_U8G2)
#include "u8g2.h"
#include "u8x8_riotos.h"

#include "images.h"
#endif

/* forward declarations */
int reset(int argc, char **argv);
int enable(int argc, char **argv);
int load(int argc, char **argv);
int store(int argc, char **argv);
int calibrate(int argc, char **argv);
int zero(int argc, char **argv);
int info(int argc, char **argv);
int level(int argc, char **argv);
int temperature(int argc, char **argv);
int level_raw(int argc, char **argv);
int temperature_raw(int argc, char **argv);
int config(int argc, char **argv);
int level_config(int argc, char **argv);
int temperature_config(int argc, char **argv);

#if IS_ACTIVE(MODULE_U8G2)
int monitor(int argc, char **argv);

static u8g2_t u8g2;
#endif

static water_sensor_params_t params = {
    .i2c_dev = I2C_DEV(0),
    .address = WATER_SENSOR_I2C_ADDRESS
};

static water_sensor_t dev;
static water_sensor_info_t dev_info;

static const shell_command_t shell_commands[] = {
    { "reset", "Reset water sensor", reset },
    { "enable", "Enable water sensor", enable },
    { "load", "Load configuration command", load },
    { "store", "Store configuration command", store },
    { "calibrate", "Calibrate water sensor", calibrate },
    { "zero", "Zero min/max", zero },
    { "info", "Read water sensor info", info },
    { "level", "Read the level", level },
    { "temperature", "Read the temperature", temperature },
    { "level_raw", "Read the level raw", level_raw },
    { "temperature_raw", "Read the temperature raw", temperature_raw },
    { "config", "Read or write global config", config },
    { "level_config", "Read or write level config", level_config },
    { "temperature_config", "Read or write temperature config", temperature_config },
#if IS_ACTIVE(MODULE_U8G2)
    { "monitor", "Enable monitor mode on LCD", monitor },
#endif
    { NULL, NULL, NULL }
};

int reset(int argc, char **argv)
{
    (void)argc;
    (void)argv;

    int result = water_sensor_reset(&dev);

    if (result != WATER_SENSOR_OK) {
        printf("error: return code %d\n", result);
        return 1;
    }

    return 0;
}

int enable(int argc, char **argv)
{
    (void)argc;
    (void)argv;

    int result = water_sensor_enable(&dev);

    if (result != WATER_SENSOR_OK) {
        printf("error: return code %d\n", result);
        return 1;
    }

    return 0;
}

int load(int argc, char **argv)
{
    (void)argc;
    (void)argv;

    int result = water_sensor_load(&dev);

    if (result != WATER_SENSOR_OK) {
        printf("error: return code %d\n", result);
        return 1;
    }

    return 0;
}

int store(int argc, char **argv)
{
    (void)argc;
    (void)argv;

    int result = water_sensor_store(&dev);

    if (result != WATER_SENSOR_OK) {
        printf("error: return code %d\n", result);
        return 1;
    }

    return 0;
}

int calibrate(int argc, char **argv)
{
    (void)argc;
    (void)argv;

    int result = water_sensor_calibrate(&dev);

    if (result != WATER_SENSOR_OK) {
        printf("error: return code %d\n", result);
        return 1;
    }

    return 0;
}

int zero(int argc, char **argv)
{
    (void)argc;
    (void)argv;

    int result = water_sensor_zero(&dev);

    if (result != WATER_SENSOR_OK) {
        printf("error: return code %d\n", result);
        return 1;
    }

    return 0;
}

int info(int argc, char **argv)
{
    (void)argc;
    (void)argv;

    water_sensor_info_t info;

    int result = water_sensor_read_info(&dev, &info);

    if (result != WATER_SENSOR_OK) {
        printf("error: return code %d\n", result);
        return 1;
    }

    printf("Identifier: %02x\n", info.id);
    printf("Level channels: %d\n", info.level_channels);
    printf("Temperature channels: %d\n", info.temperature_channels);
    printf("Enabled: %s\n", info.enabled ? "Y" : "N");

    printf("Errors: ");

    if (info.errors) {
        char *delimeter = "";

        if (info.errors & (1 << WATER_SENSOR_INFO_ERRORS_INIT)) {
            printf("%sinit", delimeter);
            delimeter = ", ";
        }
        if (info.errors & (1 << WATER_SENSOR_INFO_ERRORS_RESET)) {
            printf("%sreset", delimeter);
            delimeter = ", ";
        }
        if (info.errors & (1 << WATER_SENSOR_INFO_ERRORS_ENABLE)) {
            printf("%senable", delimeter);
            delimeter = ", ";
        }
        if (info.errors & (1 << WATER_SENSOR_INFO_ERRORS_READ)) {
            printf("%sread", delimeter);
            delimeter = ", ";
        }
        if (info.errors & (1 << WATER_SENSOR_INFO_ERRORS_ZERO)) {
            printf("%szero", delimeter);
            delimeter = ", ";
        }

        printf("\n");
    }
    else {
        printf("none\n");
    }

    printf("Context: %s\n", info.context ? "Y" : "N");

    return 0;
}

int level(int argc, char **argv)
{
    (void)argc;
    (void)argv;

    water_sensor_level_t level;

    int result = water_sensor_read_level(&dev, &level);

    if (result != WATER_SENSOR_OK) {
        printf("error: return code %d\n", result);
        return 1;
    }

    printf("Level: %d\n", level.value);
    printf("Channel: %d\n", level.channel);
    printf("Valid: %s\n", level.valid ? "Y" : "N");

    return 0;
}

int temperature(int argc, char **argv)
{
    (void)argc;
    (void)argv;

    water_sensor_temperature_t temperature;

    int result = water_sensor_read_temperature(&dev, &temperature);

    if (result != WATER_SENSOR_OK) {
        printf("error: return code %d\n", result);
        return 1;
    }

    printf("Temperature: %d\n", temperature.value);
    printf("Channel: %d\n", temperature.channel);
    printf("Valid: %s\n", temperature.valid ? "Y" : "N");

    return 0;
}

int level_raw(int argc, char **argv)
{
    unsigned start, stop;

    if (argc == 1) {
        start = 0;
        stop = dev_info.level_channels;
    } else if (argc == 2) {
        int channel = atoi(argv[1]);

        start = channel;
        stop = channel + 1;
    } else {
        printf("usage: %s [<channel>]\n", argv[0]);
        return 0;
    }

    water_sensor_level_raw_t level_raw;
    water_sensor_level_config_t level_config;

    printf("Channel\tValue\tMin\tMax\tDelta\tValid\tState\n");

    for (unsigned i = start; i < stop; i++) {
        int result = water_sensor_read_level_raw(&dev, i, &level_raw);

        if (result != WATER_SENSOR_OK) {
            printf("error: return code %d\n", result);
            return 1;
        }

        result = water_sensor_read_level_config(&dev, i, &level_config);

        if (result != WATER_SENSOR_OK) {
            printf("error: return code %d\n", result);
            return 1;
        }

        if (level_config.enabled) {
            printf("%02d\t%d\t%d\t%d\t%d\t%s\t%s\n", i, level_raw.value, level_raw.min, level_raw.max, level_raw.max - level_raw.min, level_raw.valid ? "Y" : "N", level_raw.value > level_config.offset ? "Y" : "N");
        }
        else {
            printf("%02d\t--\t--\t--\n", i);
        }
    }

    return 0;
}

int temperature_raw(int argc, char **argv)
{
    unsigned start, stop;

    if (argc == 1) {
        start = 0;
        stop = dev_info.temperature_channels;
    } else if (argc == 2) {
        int channel = atoi(argv[1]);

        start = channel;
        stop = channel + 1;
    } else {
        printf("usage: %s [<channel>]\n", argv[0]);
        return 0;
    }

    water_sensor_temperature_raw_t temperature_raw;
    water_sensor_temperature_config_t temperature_config;

    printf("Channel\tValue\tMin\tMax\tDelta\tValid\n");

    for (unsigned i = start; i < stop; i++) {
        int result = water_sensor_read_temperature_raw(&dev, i, &temperature_raw);

        if (result != WATER_SENSOR_OK) {
            printf("error: return code %d\n", result);
            return 1;
        }

        result = water_sensor_read_temperature_config(&dev, i, &temperature_config);

        if (result != WATER_SENSOR_OK) {
            printf("error: return code %d\n", result);
            return 1;
        }

        if (temperature_config.enabled) {
            printf("%02d\t%d\t%d\t%d\t%d\t%s\n", i, temperature_raw.value, temperature_raw.min, temperature_raw.max, temperature_raw.max - temperature_raw.min, temperature_raw.valid ? "Y" : "N");
        }
        else {
            printf("%02d\t--\t--\t--\t--\t--\n", i);
        }
    }

    return 0;
}

int config_get(int argc, char **argv)
{
    (void) argc;
    (void) argv;

    water_sensor_config_t config;

    int result = water_sensor_read_config(&dev, &config);

    if (result != WATER_SENSOR_OK) {
        printf("error: return code %d\n", result);
        return 1;
    }

    printf("Default level: %d\n", config.default_level);

    return 0;
}

int config_set(int argc, char **argv)
{
    if (argc < 3) {
        printf("usage: %s %s <default level>\n", argv[0], argv[1]);
        return 0;
    }

    water_sensor_config_t config;

    config.default_level = atoi(argv[2]);

    int result = water_sensor_write_config(&dev, &config);

    if (result != WATER_SENSOR_OK) {
        printf("error: return code %d\n", result);
        return 1;
    }

    return 0;
}

int config(int argc, char **argv)
{
    if (argc < 2) {
        printf("usage: %s <get|set>\n", argv[0]);
        return 0;
    }

    if (strcmp(argv[1], "get") == 0) {
        return config_get(argc, argv);
    } else if (strcmp(argv[1], "set") == 0) {
        return config_set(argc, argv);
    } else {
        printf("error: '%s' not supported\n", argv[1]);
        return 1;
    }

    return 0;
}

int level_config_get(int argc, char **argv)
{
    unsigned start, stop;

    if (argc == 2) {
        start = 0;
        stop = dev_info.level_channels;
    } else if (argc == 3) {
        int channel = atoi(argv[2]);

        start = channel;
        stop = channel + 1;
    } else {
        printf("usage: %s %s [<channel>]\n", argv[0], argv[1]);
        return 0;
    }

    water_sensor_level_config_t config;

    printf("Channel\tEnabled\tSamples\tAlpha\tOffset\tLevel\n");

    for (unsigned i = start; i < stop; i++) {
        int result = water_sensor_read_level_config(&dev, i, &config);

        if (result != WATER_SENSOR_OK) {
            printf("error: return code %d\n", result);
            return 1;
        }

        printf("%02d\t%s\t%d\t%d\t%d\t%d\n", i, config.enabled ? "Y" : "N", config.samples, config.alpha, config.offset, config.level);
    }

    return 0;
}

int level_config_set(int argc, char **argv)
{
    if (argc < 8) {
        printf("usage: %s %s <channel> <enabled> <samples> <alpha> <offset> <level>\n", argv[0], argv[1]);
        return 0;
    }

    int channel = atoi(argv[2]);

    water_sensor_level_config_t config;

    config.enabled = atoi(argv[3]);
    config.samples = atoi(argv[4]);
    config.alpha = atoi(argv[5]);
    config.offset = atoi(argv[6]);
    config.level = atoi(argv[7]);

    int result = water_sensor_write_level_config(&dev, channel, &config);

    if (result != WATER_SENSOR_OK) {
        printf("error: return code %d\n", result);
        return 1;
    }

    return 0;
}

int level_config(int argc, char **argv)
{
    if (argc < 2) {
        printf("usage: %s <get|set>\n", argv[0]);
        return 0;
    }

    if (strcmp(argv[1], "get") == 0) {
        return level_config_get(argc, argv);
    } else if (strcmp(argv[1], "set") == 0) {
        return level_config_set(argc, argv);
    } else {
        printf("error: '%s' not supported\n", argv[1]);
        return 1;
    }

    return 0;
}

int temperature_config_get(int argc, char **argv)
{
    unsigned start, stop;

    if (argc == 2) {
        start = 0;
        stop = dev_info.temperature_channels;
    } else if (argc == 3) {
        int channel = atoi(argv[2]);

        start = channel;
        stop = channel + 1;
    } else {
        printf("usage: %s %s [<channel>]\n", argv[0], argv[1]);
        return 0;
    }

    water_sensor_temperature_config_t config;

    printf("Channel\tEnabled\tAlpha\tReference\n");

    for (unsigned i = start; i < stop; i++) {
        int result = water_sensor_read_temperature_config(&dev, i, &config);

        if (result != WATER_SENSOR_OK) {
            printf("error: return code %d\n", result);
            return 1;
        }

        printf("%02d\t%s\t%d\t%d\n", i, config.enabled ? "Y" : "N", config.alpha, config.reference);
    }

    return 0;
}

int temperature_config_set(int argc, char **argv)
{
    if (argc < 6) {
        printf("usage: %s %s <channel> <enabled> <alpha> <reference>\n", argv[0], argv[1]);
        return 0;
    }

    int channel = atoi(argv[2]);

    water_sensor_temperature_config_t config;

    config.enabled = atoi(argv[3]);
    config.alpha = atoi(argv[4]);
    config.reference = atoi(argv[5]);

    int result = water_sensor_write_temperature_config(&dev, channel, &config);

    if (result != WATER_SENSOR_OK) {
        printf("error: return code %d\n", result);
        return 1;
    }

    return 0;
}

int temperature_config(int argc, char **argv)
{
    if (argc < 2) {
        printf("usage: %s <get|set>\n", argv[0]);
        return 0;
    }

    if (strcmp(argv[1], "get") == 0) {
        return temperature_config_get(argc, argv);
    } else if (strcmp(argv[1], "set") == 0) {
        return temperature_config_set(argc, argv);
    } else {
        printf("error: '%s' not supported\n", argv[1]);
        return 1;
    }

    return 0;
}

int monitor(int argc, char **argv)
{
    (void) argc;
    (void) argv;

    char buffer[3][32];

    water_sensor_level_t level;
    water_sensor_level_raw_t level_raw;
    water_sensor_temperature_t temperature;

    puts("Reset board to restart shell.");

    gpio_init(DISP_COM_PIN, GPIO_OUT);
    gpio_init(DISP_EN_PIN, GPIO_OUT);

    gpio_set(DISP_COM_PIN);
    gpio_set(DISP_EN_PIN);

    u8g2_Setup_ls013b7dh03_128x128_1(&u8g2, U8G2_R0, u8x8_byte_hw_spi_riotos, u8x8_gpio_and_delay_riotos);

    u8x8_riotos_t user_data =
    {
        .device_index = DISP_SPI,
        .pin_cs = DISP_CS_PIN,
    };

    u8g2_SetUserPtr(&u8g2, &user_data);

    u8g2_InitDisplay(&u8g2);
    u8g2_SetPowerSave(&u8g2, 0);

    while (1) {
        int result = water_sensor_read_level(&dev, &level);

        if (result != WATER_SENSOR_OK) {
            printf("error: return code %d\n", result);
        }

        result = water_sensor_read_temperature(&dev, &temperature);

        if (result != WATER_SENSOR_OK) {
            printf("error: return code %d\n", result);
        }

        snprintf(buffer[0], 32, "%i (%d, %s)", level.value, level.channel, level.valid ? "Y" : "N");
        snprintf(buffer[1], 32, "%i.%02i C (%d, %s)", temperature.value / 100, temperature.value % 100, temperature.channel, temperature.valid ? "Y" : "N");

        u8g2_FirstPage(&u8g2);

        do {
            u8g2_SetDrawColor(&u8g2, 1);
            u8g2_SetFont(&u8g2, u8g2_font_helvB12_tf);

            /* humidity */
            u8g2_DrawBitmap(&u8g2, 2, 20 - 16 + 2, 2, 16, image_rh);
            u8g2_DrawStr(&u8g2, 25, 20, buffer[0]);

            /* temperature */
            u8g2_DrawBitmap(&u8g2, 2, 40 - 16 + 2, 2, 16, image_temperature);
            u8g2_DrawStr(&u8g2, 25, 40, buffer[1]);

            u8g2_SetFont(&u8g2, u8g2_font_helvB08_tf);

            for (int i = 0; i < dev_info.level_channels; i++) {
                result = water_sensor_read_level_raw(&dev, i, &level_raw);

                if (result != WATER_SENSOR_OK) {
                    printf("error: return code %d\n", result);
                }

                snprintf(buffer[2], 32, "%d: %d (%s) %s %d/%d/%d", i, level_raw.value, level.valid ? "Y" : "N", i == level.channel ? "<--" : "   ",
                    level_raw.min, level_raw.max, level_raw.max - level_raw.min);

                u8g2_DrawStr(&u8g2, 2, 55 + (i * 9), buffer[2]);
            }

        } while (u8g2_NextPage(&u8g2));

        /* go to sleep */
        xtimer_msleep(250);
    }

    return 0;
}

int main(void)
{
    puts("Initializing sensor.");

    if (water_sensor_init(&dev, &params) != WATER_SENSOR_OK) {
        puts("Failed");
        return 1;
    }

    if (water_sensor_read_info(&dev, &dev_info) != WATER_SENSOR_OK) {
        puts("Failed");
        return 1;
    }

    printf("Detected water sensor with %d level channels and %d temperature channels.\n", dev_info.level_channels, dev_info.temperature_channels);

    /* start shell */
    char line_buf[SHELL_DEFAULT_BUFSIZE];
    shell_run(shell_commands, line_buf, SHELL_DEFAULT_BUFSIZE);

    /* should be never reached */
    return 0;
}
