/*
 * Copyright (C) 2021 Bas Stottelaar <basstottelaar@gmail.com>
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     drivers_water_sensor
 *
 * @{
 * @file
 * @brief       Internal definitions for the Water Sensor
 *
 * @author      Bas Stottelaar <basstottelaar@gmail.com>
 */

#ifndef WATER_SENSOR_INTERNALS_H
#define WATER_SENSOR_INTERNALS_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Water sensor chip addresses.
 */
#define WATER_SENSOR_I2C_ADDRESS    (0x70)

/**
 * @brief Water sensor sensor identifier
 */
#define WATER_SENSOR_ID (0xBA)

/**
 * @name Water sensor commands.
 * @{
 */
#define WATER_SENSOR_RESET      (0x00)
#define WATER_SENSOR_ENABLE     (0x01)
#define WATER_SENSOR_LOAD       (0x02)
#define WATER_SENSOR_STORE      (0x03)
#define WATER_SENSOR_CALIBRATE  (0x04)
#define WATER_SENSOR_ZERO       (0x05)
/** @} */

/**
 * @name Water sensor registers.
 * @{
 */
#define WATER_SENSOR_READ_INFO                  (0xA0)
#define WATER_SENSOR_READ_LEVEL                 (0xA1)
#define WATER_SENSOR_READ_TEMPERATURE           (0xA2)
#define WATER_SENSOR_READ_LEVEL_RAW             (0xA3)
#define WATER_SENSOR_READ_TEMPERATURE_RAW       (0xA4)
#define WATER_SENSOR_READ_CONFIG                (0xA5)
#define WATER_SENSOR_WRITE_CONFIG               (0xA6)
#define WATER_SENSOR_READ_LEVEL_CONFIG          (0xA7)
#define WATER_SENSOR_WRITE_LEVEL_CONFIG         (0xA8)
#define WATER_SENSOR_READ_TEMPERATURE_CONFIG    (0xA9)
#define WATER_SENSOR_WRITE_TEMPERATURE_CONFIG   (0xAA)
/** @} */

/**
 * @name Water sensor register sizes.
 * @{
 */
#define WATER_SENSOR_INFO_SIZE                  (6U)
#define WATER_SENSOR_LEVEL_SIZE                 (4U)
#define WATER_SENSOR_TEMPERATURE_SIZE           (4U)
#define WATER_SENSOR_LEVEL_RAW_SIZE             (7U)
#define WATER_SENSOR_TEMPERATURE_RAW_SIZE       (7U)
#define WATER_SENSOR_CONFIG_SIZE                (2U)
#define WATER_SENSOR_LEVEL_CONFIG_SIZE          (8U)
#define WATER_SENSOR_TEMPERATURE_CONFIG_SIZE    (4U)
/** @} */

/**
 * @name Water sensor error bits.
 * @{
 */
#define WATER_SENSOR_INFO_ERRORS_INIT   (0U)
#define WATER_SENSOR_INFO_ERRORS_RESET  (1U)
#define WATER_SENSOR_INFO_ERRORS_ENABLE (2U)
#define WATER_SENSOR_INFO_ERRORS_READ   (3U)
#define WATER_SENSOR_INFO_ERRORS_ZERO   (4U)
/** @} */

#ifdef __cplusplus
}
#endif

#endif /* WATER_SENSOR_INTERNALS_H */
/** @} */
