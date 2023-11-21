/*

R4_DMA.cpp  --  Use DMA controller on UNO-R4 boards.
     Copyright (C) 2023  David C.

     This program is free software: you can redistribute it and/or modify
     it under the terms of the GNU General Public License as published by
     the Free Software Foundation, either version 3 of the License, or
     (at your option) any later version.

     This program is distributed in the hope that it will be useful,
     but WITHOUT ANY WARRANTY; without even the implied warranty of
     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
     GNU General Public License for more details.

     You should have received a copy of the GNU General Public License
     along with this program.  If not, see <http://www.gnu.org/licenses/>.

     */

#include "R4_DMA.h"

DMA_Channel DMA0(0);
DMA_Channel DMA1(1);
DMA_Channel DMA2(2);
DMA_Channel DMA3(3);

void dtiHandler0() {
  DMA0.internalHandler();
}
void dtiHandler1() {
  DMA1.internalHandler();
}
void dtiHandler2() {
  DMA2.internalHandler();
}
void dtiHandler3() {
  DMA3.internalHandler();
}

void (*handlers[])() = { dtiHandler0, dtiHandler1, dtiHandler2, dtiHandler3 };

DMA_Channel::DMA_Channel(uint8_t aChannel)
  : channelIndex(aChannel) {
  R_DMAC0_Type* channels[] = { R_DMAC0, R_DMAC1, R_DMAC2, R_DMAC3 };
  if (aChannel < 4) {
    channel = channels[channelIndex];
  }
};

void DMA_Channel::internalHandler() {
  R_ICU->IELSR[eventLinkIndex] &= ~(R_ICU_IELSR_IR_Msk);
  // Clear Interrupt Flag in DMAC
  channel->DMSTS = 0x00;
  if (isrCallback) {
    isrCallback();
  }
}

void DMA_Channel::startInterrupt() {
  if (eventLinkIndex == -1) {
    dmac_extended_cfg_t cfg;
    cfg.irq = FSP_INVALID_VECTOR;
    cfg.channel = channelIndex;
    noInterrupts();

    if ((IRQManager::getInstance().addDMA(cfg, handlers[channelIndex]))) {
      eventLinkIndex = cfg.irq;
      R_BSP_IrqDisable((IRQn_Type)eventLinkIndex);
      R_BSP_IrqStatusClear((IRQn_Type)eventLinkIndex);
      NVIC_SetPriority((IRQn_Type)eventLinkIndex, 12);
      R_BSP_IrqEnable((IRQn_Type)eventLinkIndex);
    }
    interrupts();
  }
}

void DMA_Channel::attachTransferEndInterrupt(void (*isr)()) {
  if (eventLinkIndex < 0) {
    startInterrupt();
  }
  channel->DMINT = 0x10;
  isrCallback = isr;
}

void DMA_Channel::setTriggerSource(uint8_t source) {
  // diable transfers
  channel->DMCNT = 0;
  if (source) {
    R_ICU->DELSR[channelIndex] = source;
    channel->DMTMD |= 1;
  } else {
    // set as software Trigger
    channel->DMTMD &= ~1;
  }
  // enable transfer
  channel->DMCNT = 1;
}

void DMA_Channel::requestTransfer() {
  channel->DMREQ = 0x01;
}

void DMA_Channel::start(DMA_Settings* aSettings) {
  settings = aSettings;
  // disable controller
  R_DMA->DMAST = 0;
  // diable transfers
  channel->DMCNT = 0;
  //set Address Mode Register
  channel->DMAMD = (settings->sourceUpdateMode << R_DMAC0_DMAMD_SM_Pos)
                   | (settings->destUpdateMode << R_DMAC0_DMAMD_DM_Pos);
  // set transfer Mode
  channel->DMTMD = (settings->mode << R_DMAC0_DMTMD_MD_Pos)
                   | (settings->repeatAreaSelection << R_DMAC0_DMTMD_DTS_Pos)
                   | (settings->unitSize << R_DMAC0_DMTMD_SZ_Pos);
  // set source and destination address
  channel->DMSAR = settings->sourceAddress;
  channel->DMDAR = settings->destAddress;
  // set repeat size and transfer counter and block count
  switch (settings->mode) {
    case NORMAL:
      channel->DMCRA = settings->transferCount;
      channel->DMCRB = 0;
      break;
    case REPEAT:
      channel->DMCRA = (settings->transferSize << 16) | settings->transferSize;
      channel->DMCRB = settings->transferCount;
      break;
    case BLOCK:
      channel->DMCRA = (settings->transferSize << 16) | settings->transferSize;
      channel->DMCRB = settings->transferCount;
      break;
  }
  // set offset register
  channel->DMOFR = settings->addressOffset;

  // enable transfer
  channel->DMCNT = 1;
  // enable DMAC controller
  R_DMA->DMAST = 1;
}