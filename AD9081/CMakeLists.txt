# SPDX-License-Identifier: Apache-2.0

zephyr_library()

zephyr_library_sources(
  adi_ad9081_adc.c
  adi_ad9081_dac.c
  adi_ad9081_device.c
  adi_ad9081_hal.c
  adi_ad9081_jesd.c
  adi_ad9081_sync.c
)

zephyr_include_directories(.)
