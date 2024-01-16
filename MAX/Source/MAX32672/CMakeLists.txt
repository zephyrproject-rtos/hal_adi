# Makefile - Analog Devices SDK
#
# Copyright (c) 2024 Analog Devices, Inc.
#
# SPDX-License-Identifier: Apache-2.0


if(NOT TARGET_REV)
  # Default version A1, not actively use in driver but requires to be defined.
  zephyr_compile_definitions(-DTARGET_REV=0x4131)
endif()

zephyr_include_directories(
    ${MSDK_PERIPH_SRC_DIR}/ADC
    ${MSDK_PERIPH_SRC_DIR}/AES
    ${MSDK_PERIPH_SRC_DIR}/CTB
    ${MSDK_PERIPH_SRC_DIR}/DMA
    ${MSDK_PERIPH_SRC_DIR}/FLC
    ${MSDK_PERIPH_SRC_DIR}/GPIO
    ${MSDK_PERIPH_SRC_DIR}/I2C
    ${MSDK_PERIPH_SRC_DIR}/I2S
    ${MSDK_PERIPH_SRC_DIR}/ICC
    ${MSDK_PERIPH_SRC_DIR}/LP
    ${MSDK_PERIPH_SRC_DIR}/QDEC
    ${MSDK_PERIPH_SRC_DIR}/RTC
    ${MSDK_PERIPH_SRC_DIR}/SPI
    ${MSDK_PERIPH_SRC_DIR}/SYS
    ${MSDK_PERIPH_SRC_DIR}/TMR
    ${MSDK_PERIPH_SRC_DIR}/TRNG
    ${MSDK_PERIPH_SRC_DIR}/UART
    ${MSDK_PERIPH_SRC_DIR}/WDT
)

zephyr_library_sources(
    ./max32xxx_system.c

    ${MSDK_CMSIS_DIR}/Source/system_max32672.c

    ${MSDK_PERIPH_SRC_DIR}/ICC/icc_common.c
    ${MSDK_PERIPH_SRC_DIR}/ICC/icc_me21.c
    ${MSDK_PERIPH_SRC_DIR}/ICC/icc_reva.c

    ${MSDK_PERIPH_SRC_DIR}/LP/lp_me21.c

    ${MSDK_PERIPH_SRC_DIR}/SYS/mxc_assert.c
    ${MSDK_PERIPH_SRC_DIR}/SYS/mxc_delay.c
    ${MSDK_PERIPH_SRC_DIR}/SYS/mxc_lock.c
    ${MSDK_PERIPH_SRC_DIR}/SYS/pins_me21.c
    ${MSDK_PERIPH_SRC_DIR}/SYS/sys_me21.c

    ${MSDK_PERIPH_SRC_DIR}/DMA/dma_me21.c
    ${MSDK_PERIPH_SRC_DIR}/DMA/dma_reva.c

    ${MSDK_PERIPH_SRC_DIR}/QDEC/qdec_me21.c
    ${MSDK_PERIPH_SRC_DIR}/QDEC/qdec_reva.c
)

if (CONFIG_ADC_MAX32)
zephyr_library_sources(
    ${MSDK_PERIPH_SRC_DIR}/ADC/adc_me21.c
    ${MSDK_PERIPH_SRC_DIR}/ADC/adc_revb.c
)
endif()

if (CONFIG_UART_MAX32)
zephyr_library_sources(
    ${MSDK_PERIPH_SRC_DIR}/UART/uart_common.c
    ${MSDK_PERIPH_SRC_DIR}/UART/uart_me21.c
    ${MSDK_PERIPH_SRC_DIR}/UART/uart_revb.c
)
endif()

if (CONFIG_GPIO_MAX32)
zephyr_library_sources(
    ${MSDK_PERIPH_SRC_DIR}/GPIO/gpio_common.c
    ${MSDK_PERIPH_SRC_DIR}/GPIO/gpio_me21.c
    ${MSDK_PERIPH_SRC_DIR}/GPIO/gpio_reva.c
)
endif()

if (CONFIG_SPI_MAX32)
zephyr_library_sources(
    ${MSDK_PERIPH_SRC_DIR}/SPI/spi_me21.c
    ${MSDK_PERIPH_SRC_DIR}/SPI/spi_reva1.c
)
endif()

if (CONFIG_I2C_MAX32)
zephyr_library_sources(
    ${MSDK_PERIPH_SRC_DIR}/I2C/i2c_me21.c
    ${MSDK_PERIPH_SRC_DIR}/I2C/i2c_reva.c
)
endif()

if (CONFIG_I2S_MAX32)
zephyr_library_sources(
    ${MSDK_PERIPH_SRC_DIR}/I2S/i2s_me21.c
    ${MSDK_PERIPH_SRC_DIR}/I2S/i2s_reva.c
)
endif()

if (CONFIG_WDT_MAX32)
zephyr_library_sources(
    ${MSDK_PERIPH_SRC_DIR}/WDT/wdt_common.c
    ${MSDK_PERIPH_SRC_DIR}/WDT/wdt_me21.c
    ${MSDK_PERIPH_SRC_DIR}/WDT/wdt_revb.c
)
endif()

if (CONFIG_RTC_MAX32)
zephyr_library_sources(
    ${MSDK_PERIPH_SRC_DIR}/RTC/rtc_me21.c
    ${MSDK_PERIPH_SRC_DIR}/RTC/rtc_reva.c
)
endif()

if (CONFIG_SOC_FLASH_MAX32)
zephyr_library_sources(
    ${MSDK_PERIPH_SRC_DIR}/FLC/flc_common.c
    ${MSDK_PERIPH_SRC_DIR}/FLC/flc_me21.c
    ${MSDK_PERIPH_SRC_DIR}/FLC/flc_reva.c
)
endif()

if (CONFIG_PWM_MAX32 OR CONFIG_TIMER_MAX32) 
zephyr_library_sources(
    ${MSDK_PERIPH_SRC_DIR}/TMR/tmr_common.c
    ${MSDK_PERIPH_SRC_DIR}/TMR/tmr_me21.c
    ${MSDK_PERIPH_SRC_DIR}/TMR/tmr_revb.c
)
endif()

if (CONFIG_ENTROPY_MAX32_TRNG)
zephyr_library_sources(
    ${MSDK_PERIPH_SRC_DIR}/TRNG/trng_me21.c
    ${MSDK_PERIPH_SRC_DIR}/TRNG/trng_revb.c
)
endif()

if (CONFIG_CRYPTO_MAX32)
zephyr_library_sources(
    ${MSDK_PERIPH_SRC_DIR}/AES/aes_me21.c
    ${MSDK_PERIPH_SRC_DIR}/AES/aes_revb.c

    ${MSDK_PERIPH_SRC_DIR}/CTB/ctb_me21.c
    ${MSDK_PERIPH_SRC_DIR}/CTB/ctb_reva.c
    ${MSDK_PERIPH_SRC_DIR}/CTB/ctb_common.c
)
endif()