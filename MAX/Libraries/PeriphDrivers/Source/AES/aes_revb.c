/******************************************************************************
 *
 * Copyright (C) 2022-2023 Maxim Integrated Products, Inc. (now owned by 
 * Analog Devices, Inc.),
 * Copyright (C) 2023-2024 Analog Devices, Inc.
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

#include <stdlib.h>
#include <string.h>
#include "mxc_sys.h"
#include "mxc_device.h"
#include "mxc_errors.h"
#include "mxc_assert.h"
#include "mxc_lock.h"
#include "dma.h"
#include "aes_regs.h"
#include "aes_revb.h"
#include "trng_revb.h"

/* **** Variable Declaration **** */
typedef struct {
    uint8_t enc;
    mxc_dma_regs_t *dma;
    uint8_t channelRX;
    uint8_t channelTX;
    uint32_t remain;
    uint32_t *inputText;
    uint32_t *outputText;
} mxc_aes_revb_dma_req_t;

static mxc_aes_revb_dma_req_t dma_state;

#define SWAP_BYTES(x)                                                                     \
    ((((x) >> 24) & 0x000000FF) | (((x) >> 8) & 0x0000FF00) | (((x) << 8) & 0x00FF0000) | \
     (((x) << 24) & 0xFF000000))

/* Prevent GCC from optimimzing this function to memcpy */
static void __attribute__((optimize("no-tree-loop-distribute-patterns")))
memcpy32r(uint32_t *dst, const uint32_t *src, unsigned int len)
{
    uint32_t *dstr = dst + (len / 4) - 1;
    while (len) {
        *dstr = SWAP_BYTES(*src);
        dstr--;
        src++;
        len -= 4;
    }
}

int MXC_AES_RevB_Init(mxc_aes_revb_regs_t *aes, mxc_dma_regs_t *dma)
{
    aes->ctrl = 0x00;

    while (MXC_AES_RevB_IsBusy(aes) != E_NO_ERROR) {}

    aes->ctrl |= MXC_F_AES_REVB_CTRL_EN;

    dma_state.dma = dma;

    return E_NO_ERROR;
}

int MXC_AES_RevB_Shutdown(mxc_aes_revb_regs_t *aes)
{
    MXC_AES_RevB_FlushInputFIFO(aes);
    MXC_AES_RevB_FlushOutputFIFO(aes);

    while (MXC_AES_RevB_IsBusy(aes) != E_NO_ERROR) {}

    aes->ctrl = 0x00;

    return E_NO_ERROR;
}

int MXC_AES_RevB_IsBusy(mxc_aes_revb_regs_t *aes)
{
    if (aes->status & MXC_F_AES_REVB_STATUS_BUSY) {
        return E_BUSY;
    }

    return E_NO_ERROR;
}

void MXC_AES_RevB_SetKeySize(mxc_aes_revb_regs_t *aes, mxc_aes_revb_keys_t key)
{
    while (MXC_AES_IsBusy() != E_NO_ERROR) {}
    aes->ctrl |= key;
}

mxc_aes_keys_t MXC_AES_RevB_GetKeySize(mxc_aes_revb_regs_t *aes)
{
    return (aes->ctrl & MXC_F_AES_REVB_CTRL_KEY_SIZE);
}

void MXC_AES_RevB_FlushInputFIFO(mxc_aes_revb_regs_t *aes)
{
    while (MXC_AES_IsBusy() != E_NO_ERROR) {}
    aes->ctrl |= MXC_F_AES_REVB_CTRL_INPUT_FLUSH;
}

void MXC_AES_RevB_FlushOutputFIFO(mxc_aes_revb_regs_t *aes)
{
    while (MXC_AES_IsBusy() != E_NO_ERROR) {}
    aes->ctrl |= MXC_F_AES_REVB_CTRL_OUTPUT_FLUSH;
}

void MXC_AES_RevB_Start(mxc_aes_revb_regs_t *aes)
{
    while (MXC_AES_IsBusy() != E_NO_ERROR) {}
    aes->ctrl |= MXC_F_AES_REVB_CTRL_START;
}

void MXC_AES_RevB_EnableInt(mxc_aes_revb_regs_t *aes, uint32_t interrupt)
{
    aes->inten |= (interrupt & (MXC_F_AES_REVB_INTEN_DONE | MXC_F_AES_REVB_INTEN_KEY_CHANGE |
                                MXC_F_AES_REVB_INTEN_KEY_ZERO | MXC_F_AES_REVB_INTEN_OV));
}

void MXC_AES_RevB_DisableInt(mxc_aes_revb_regs_t *aes, uint32_t interrupt)
{
    aes->inten &= ~(interrupt & (MXC_F_AES_REVB_INTEN_DONE | MXC_F_AES_REVB_INTEN_KEY_CHANGE |
                                 MXC_F_AES_REVB_INTEN_KEY_ZERO | MXC_F_AES_REVB_INTEN_OV));
}

uint32_t MXC_AES_RevB_GetFlags(mxc_aes_revb_regs_t *aes)
{
    return aes->intfl;
}

void MXC_AES_RevB_ClearFlags(mxc_aes_revb_regs_t *aes, uint32_t flags)
{
    aes->intfl = (flags & (MXC_F_AES_REVB_INTFL_DONE | MXC_F_AES_REVB_INTFL_KEY_CHANGE |
                           MXC_F_AES_REVB_INTFL_KEY_ZERO | MXC_F_AES_REVB_INTFL_OV));
}

int MXC_AES_RevB_Generic(mxc_aes_revb_regs_t *aes, mxc_aes_revb_req_t *req)
{
    int i;
    int remain;

    if (req == NULL) {
        return E_NULL_PTR;
    }

    if (req->inputData == NULL || req->resultData == NULL) {
        return E_NULL_PTR;
    }

    if (req->length == 0) {
        return E_BAD_PARAM;
    }

    remain = req->length;

    MXC_AES_RevB_FlushInputFIFO(aes);
    MXC_AES_RevB_FlushOutputFIFO(aes);

    MXC_AES_RevB_SetKeySize(aes, req->keySize);

    while (MXC_AES_IsBusy() != E_NO_ERROR) {}

    MXC_SETFIELD(aes->ctrl, MXC_F_AES_REVB_CTRL_TYPE,
                 req->encryption << MXC_F_AES_REVB_CTRL_TYPE_POS);

    while (remain / 4) {
        for (i = 0; i < 4; i++) {
            aes->fifo = SWAP_BYTES(req->inputData[3 - i]);
        }
        req->inputData += 4;

        while (!(aes->intfl & MXC_F_AES_REVB_INTFL_DONE)) {}
        aes->intfl |= MXC_F_AES_REVB_INTFL_DONE;

        for (i = 0; i < 4; i++) {
            uint32_t tmp = aes->fifo;
            req->resultData[3 - i] = SWAP_BYTES(tmp);
        }
        req->resultData += 4;

        remain -= 4;
    }

    if (remain % 4) {
        for (i = 0; i < remain; i++) {
            aes->fifo = SWAP_BYTES(req->inputData[remain - 1 - i]);
        }
        req->inputData += remain;

        // Pad last block with 0's
        for (i = remain; i < 4; i++) {
            aes->fifo = 0;
        }

        while (!(aes->intfl & MXC_F_AES_REVB_INTFL_DONE)) {}
        aes->intfl |= MXC_F_AES_REVB_INTFL_DONE;

        for (i = 0; i < 4; i++) {
            uint32_t tmp = aes->fifo;
            req->resultData[3 - i] = SWAP_BYTES(tmp);
        }
        req->resultData += 4;
    }
    return E_NO_ERROR;
}

int MXC_AES_RevB_Encrypt(mxc_aes_revb_regs_t *aes, mxc_aes_revb_req_t *req)
{
    return MXC_AES_RevB_Generic(aes, req);
}

int MXC_AES_RevB_Decrypt(mxc_aes_revb_regs_t *aes, mxc_aes_revb_req_t *req)
{
    return MXC_AES_RevB_Generic(aes, req);
}

int MXC_AES_RevB_TXDMAConfig(void *src_addr, int len, mxc_dma_regs_t *dma)
{
    uint8_t channel;
    mxc_dma_config_t config;
    mxc_dma_srcdst_t srcdst;

    if (src_addr == NULL) {
        return E_NULL_PTR;
    }

    if (len == 0) {
        return E_BAD_PARAM;
    }

#if (TARGET_NUM == 32657)
    MXC_DMA_Init(dma);

    channel = MXC_DMA_AcquireChannel(dma);
#else
    MXC_DMA_Init();

    channel = MXC_DMA_AcquireChannel();
#endif

    dma_state.channelTX = channel;

    config.reqsel = MXC_DMA_REQUEST_AESTX;

    config.ch = channel;

    config.srcwd = MXC_DMA_WIDTH_WORD;
    config.dstwd = MXC_DMA_WIDTH_WORD;

    config.srcinc_en = 1;
    config.dstinc_en = 0;

    srcdst.ch = channel;
    srcdst.source = src_addr;

    if (dma_state.enc == 1) {
        srcdst.len = 4;
    } else if (len > 4) {
        srcdst.len = 4;
    } else {
        srcdst.len = len;
    }

    MXC_DMA_ConfigChannel(config, srcdst);
    MXC_DMA_SetCallback(channel, MXC_AES_RevB_DMACallback);

#if (TARGET_NUM == 32657)
    MXC_DMA_EnableInt(dma, channel);
#else
    MXC_DMA_EnableInt(channel);
#endif

    MXC_DMA_Start(channel);
    //MXC_DMA->ch[channel].ctrl |= MXC_F_DMA_CTRL_CTZ_IE;
    MXC_DMA_SetChannelInterruptEn(channel, 0, 1);

    return E_NO_ERROR;
}

int MXC_AES_RevB_RXDMAConfig(void *dest_addr, int len, mxc_dma_regs_t *dma)
{
    if (dest_addr == NULL) {
        return E_NULL_PTR;
    }

    if (len == 0) {
        return E_BAD_PARAM;
    }

    uint8_t channel;
    mxc_dma_config_t config;
    mxc_dma_srcdst_t srcdst;

#if (TARGET_NUM == 32657)
    MXC_DMA_Init(dma);

    channel = MXC_DMA_AcquireChannel(dma);
#else
    MXC_DMA_Init();

    channel = MXC_DMA_AcquireChannel();
#endif

    dma_state.channelRX = channel;

    config.reqsel = MXC_DMA_REQUEST_AESRX;

    config.ch = channel;

    config.srcwd = MXC_DMA_WIDTH_WORD;
    config.dstwd = MXC_DMA_WIDTH_WORD;

    config.srcinc_en = 0;
    config.dstinc_en = 1;

    srcdst.ch = channel;
    srcdst.dest = dest_addr;

    if (dma_state.enc == 0) {
        srcdst.len = 4;
    } else if (len > 4) {
        srcdst.len = 4;
    } else {
        srcdst.len = len;
    }

    MXC_DMA_ConfigChannel(config, srcdst);
    MXC_DMA_SetCallback(channel, MXC_AES_RevB_DMACallback);

#if (TARGET_NUM == 32657)
    MXC_DMA_EnableInt(dma, channel);
#else
    MXC_DMA_EnableInt(channel);
#endif

    MXC_DMA_Start(channel);
    //MXC_DMA->ch[channel].ctrl |= MXC_F_DMA_CTRL_CTZ_IE;
    MXC_DMA_SetChannelInterruptEn(channel, 0, 1);

    return E_NO_ERROR;
}

int MXC_AES_RevB_GenericAsync(mxc_aes_revb_regs_t *aes, mxc_aes_revb_req_t *req, uint8_t enc)
{
    if (req == NULL) {
        return E_NULL_PTR;
    }

    if (req->inputData == NULL || req->resultData == NULL) {
        return E_NULL_PTR;
    }

    if (req->length == 0) {
        return E_BAD_PARAM;
    }

    MXC_AES_RevB_FlushInputFIFO(aes);
    MXC_AES_RevB_FlushOutputFIFO(aes);

    MXC_AES_RevB_SetKeySize(aes, req->keySize);

    MXC_AES_IsBusy();
    MXC_SETFIELD(aes->ctrl, MXC_F_AES_REVB_CTRL_TYPE,
                 req->encryption << MXC_F_AES_REVB_CTRL_TYPE_POS);

    dma_state.enc = enc;
    dma_state.remain = req->length;
    dma_state.inputText = req->inputData;
    dma_state.outputText = req->resultData;

    aes->ctrl |= MXC_F_AES_REVB_CTRL_DMA_RX_EN; //Enable AES DMA
    aes->ctrl |= MXC_F_AES_REVB_CTRL_DMA_TX_EN; //Enable AES DMA

    if (MXC_AES_RevB_TXDMAConfig(dma_state.inputText, dma_state.remain, dma_state.dma) !=
        E_NO_ERROR) {
        return E_BAD_PARAM;
    }

    return E_NO_ERROR;
}

int MXC_AES_RevB_EncryptAsync(mxc_aes_revb_regs_t *aes, mxc_aes_revb_req_t *req)
{
    return MXC_AES_RevB_GenericAsync(aes, req, 0);
}

int MXC_AES_RevB_DecryptAsync(mxc_aes_revb_regs_t *aes, mxc_aes_revb_req_t *req)
{
    return MXC_AES_RevB_GenericAsync(aes, req, 1);
}

void MXC_AES_RevB_DMACallback(int ch, int error)
{
    if (error != E_NO_ERROR) {
    } else {
        if (dma_state.channelTX == ch) {
            MXC_DMA_ReleaseChannel(dma_state.channelTX);
            if (dma_state.remain < 4) {
                MXC_AES_Start();
            }
            MXC_AES_RevB_RXDMAConfig(dma_state.outputText, dma_state.remain, dma_state.dma);
        } else if (dma_state.channelRX == ch) {
            if (dma_state.remain > 4) {
                dma_state.remain -= 4;
            } else if (dma_state.remain > 0) {
                dma_state.remain = 0;
            }
            MXC_DMA_ReleaseChannel(dma_state.channelRX);
            if (dma_state.remain > 0) {
                MXC_AES_RevB_TXDMAConfig(dma_state.inputText, dma_state.remain, dma_state.dma);
            }
        }
    }
}

void MXC_AES_RevB_SetExtKey(mxc_aeskeys_revb_regs_t *aeskeys, const void *key, mxc_aes_keys_t len)
{
    int numBytes;

    if (len == MXC_AES_128BITS) {
        numBytes = 16;
    } else if (len == MXC_AES_192BITS) {
        numBytes = 24;
    } else {
        numBytes = 32;
    }

    /* TODO: Figure out if this is the correct byte ordering */
    memcpy32r((void *)&(aeskeys->key0), key, numBytes);
}
