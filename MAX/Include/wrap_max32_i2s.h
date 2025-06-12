/******************************************************************************
 *
 * Copyright (C) 2025 Croxel, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 ******************************************************************************/

#ifndef LIBRARIES_ZEPHYR_MAX_INCLUDE_WRAP_MAX32_I2S_H_
#define LIBRARIES_ZEPHYR_MAX_INCLUDE_WRAP_MAX32_I2S_H_

/***** Includes *****/
#include <i2s.h>

#ifdef __cplusplus
extern "C"
{
#endif

#if defined(CONFIG_SOC_MAX32655) || defined(CONFIG_SOC_MAX32662) || defined(CONFIG_SOC_MAX32672) || defined(CONFIG_SOC_MAX32675) || defined(CONFIG_SOC_MAX32680) || defined(CONFIG_SOC_MAX32690) || defined(CONFIG_SOC_MAX78000) || defined(CONFIG_SOC_MAX78002)

	static inline uint32_t Wrap_MXC_I2S_CalculateClockDiv(uint32_t sampleRate, uint16_t wordSize)
	{
		return MXC_I2S_CalculateClockDiv(sampleRate, wordSize) + 1;
	}

	static inline int Wrap_MXC_I2S_Init(mxc_i2s_regs_t *i2s, mxc_i2s_req_t *req)
	{
		/*
		 * MXC_I2S_Init internally required atleast one data pointer set
		 * but we don't have any data to send or receive at this point.
		 * So we set a dummy buffer to satisfy the requirement.
		 * This buffer will not be used in the transaction.
		 */
		uint32_t buf0[1] = {0};
		req->rxData = buf0;
		req->length = sizeof(buf0) / sizeof(buf0[0]);

		return MXC_I2S_Init(req);
	}

	static inline void Wrap_MXC_I2S_EnableDMATx(mxc_i2s_regs_t *i2s)
	{
		i2s->dmach0 |= MXC_F_I2S_DMACH0_DMA_TX_EN;
		i2s->ctrl0ch0 |= MXC_F_I2S_CTRL0CH0_TX_EN;
	}

	static inline void Wrap_MXC_I2S_EnableDMARx(mxc_i2s_regs_t *i2s)
	{
		i2s->dmach0 |= MXC_F_I2S_DMACH0_DMA_RX_EN;
		i2s->ctrl0ch0 |= MXC_F_I2S_CTRL0CH0_RX_EN;
	}

	static inline void Wrap_MXC_I2S_SetDMATxThreshold(mxc_i2s_regs_t *i2s, uint8_t threshold)
	{
		i2s->dmach0 &= ~MXC_F_I2S_DMACH0_DMA_TX_THD_VAL;
		i2s->dmach0 |= (threshold << MXC_F_I2S_DMACH0_DMA_TX_THD_VAL_POS);
	}

	static inline void Wrap_MXC_I2S_SetDMARxThreshold(mxc_i2s_regs_t *i2s, uint8_t threshold)
	{
		i2s->dmach0 &= ~MXC_F_I2S_DMACH0_DMA_RX_THD_VAL;
		i2s->dmach0 |= (threshold << MXC_F_I2S_DMACH0_DMA_RX_THD_VAL_POS);
	}

#else
/* CONFIG_SOC_MAX32650 ||  CONFIG_SOC_MAX32660 */
#error "Unsupported SoC for wrap_max32_i2s.h"

#endif /* CONFIG_SOC_MAX32655 || CONFIG_SOC_MAX32662 || CONFIG_SOC_MAX32672 || CONFIG_SOC_MAX32675 || CONFIG_SOC_MAX32680 || CONFIG_SOC_MAX32690 || CONFIG_SOC_MAX78000 || CONFIG_SOC_MAX78002 */

#ifdef __cplusplus
}
#endif

#endif /* LIBRARIES_ZEPHYR_MAX_INCLUDE_WRAP_MAX32_I2S_H_ */