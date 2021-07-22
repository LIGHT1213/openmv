/*
 * This file is part of the OpenMV project.
 *
 * Copyright (c) 2013-2021 Ibrahim Abdelkader <iabdalkader@openmv.io>
 * Copyright (c) 2013-2021 Kwabena W. Agyeman <kwagyeman@openmv.io>
 *
 * This work is licensed under the MIT license, see the file LICENSE for details.
 *
 * NINA-W10 driver BSP implementation.
 */
#if MICROPY_PY_NINAW10
#include <stdint.h>
#include <string.h>
#include "py/mphal.h"

#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "omv_boardconfig.h"

#include "nina.h"
#include "nina_bsp.h"

int nina_bsp_init()
{
    gpio_init(WIFI_CS_PIN);
    gpio_set_dir(WIFI_CS_PIN, GPIO_OUT);

    gpio_init(WIFI_ACK_PIN);
    gpio_set_dir(WIFI_ACK_PIN, GPIO_IN);

    gpio_init(WIFI_RST_PIN);
    gpio_set_dir(WIFI_RST_PIN, GPIO_OUT);

    gpio_init(WIFI_GPIO0_PIN);
    gpio_set_dir(WIFI_GPIO0_PIN, GPIO_OUT);

    gpio_put(WIFI_GPIO0_PIN, 1);
    gpio_put(WIFI_CS_PIN, 1);

    gpio_put(WIFI_RST_PIN, 0);
    mp_hal_delay_ms(10);
    gpio_put(WIFI_RST_PIN, 1);
    mp_hal_delay_ms(750);

    gpio_put(WIFI_GPIO0_PIN, 0);

    spi_init(WIFI_SPI, 8 * 1000 * 1000);

    gpio_set_function(WIFI_SCLK_PIN, GPIO_FUNC_SPI);
    gpio_set_function(WIFI_MOSI_PIN, GPIO_FUNC_SPI);
    
    return 0;
}

int nina_bsp_reset()
{
    return 0;
}

int nina_bsp_spi_slave_select(uint32_t timeout)
{
    // Wait for ACK to go low.
    for (mp_uint_t start = mp_hal_ticks_ms(); gpio_get(WIFI_ACK_PIN) != 0; mp_hal_delay_ms(1)) {
        if ((mp_hal_ticks_ms() - start) >= timeout) {
            return -1;
        }
    }

    // Chip select.
    gpio_put(WIFI_CS_PIN, 0);

    // Wait for ACK to go high.
    for (mp_uint_t start = mp_hal_ticks_ms(); gpio_get(WIFI_ACK_PIN) == 0; mp_hal_delay_ms(1)) {
        if ((mp_hal_ticks_ms() - start) >= 100) {
            gpio_put(WIFI_CS_PIN, 1);
            return -1;
        }
    }

    return 0;
}

int nina_bsp_spi_slave_deselect()
{
    gpio_put(WIFI_CS_PIN, 1);
    return 0;
}

int nina_bsp_spi_transfer(const uint8_t *tx_buf, uint8_t *rx_buf, uint32_t size)
{
    gpio_put(WIFI_CS_PIN, 0);
    if (tx_buf && rx_buf) {
        spi_write_read_blocking(WIFI_SPI, tx_buf, rx_buf, size);
    } else if (tx_buf) {
        spi_write_blocking(WIFI_SPI, tx_buf, size);
    } else if (rx_buf) {
        spi_read_blocking(WIFI_SPI, 0x00, rx_buf, size);
    }
    gpio_put(WIFI_CS_PIN, 1);
    return 0;
}
#endif //MICROPY_PY_NINAW10
