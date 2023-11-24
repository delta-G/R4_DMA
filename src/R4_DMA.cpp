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

DMA_Channel *DMA_Channel::instances[4] = { NULL, NULL, NULL, NULL };

void dtiHandler0() {
	if (DMA_Channel::instances[0]) {
		DMA_Channel::instances[0]->internalHandler();
	}
}
void dtiHandler1() {
	if (DMA_Channel::instances[1]) {
		DMA_Channel::instances[1]->internalHandler();
	}
}
void dtiHandler2() {
	if (DMA_Channel::instances[2]) {
		DMA_Channel::instances[2]->internalHandler();
	}
}
void dtiHandler3() {
	if (DMA_Channel::instances[3]) {
		DMA_Channel::instances[3]->internalHandler();
	}
}

void (*handlers[])() = {dtiHandler0, dtiHandler1, dtiHandler2, dtiHandler3};

bool DMA_Channel::getChannel() {
	R_DMAC0_Type *channels[] = { R_DMAC0, R_DMAC1, R_DMAC2, R_DMAC3 };
	bool rv = false;
	if (hasChannel()) {
		rv = true;
	} else {
		for (int i = 0; i < 4; i++) {
			if (!instances[i]) {
				channel = channels[i];
				channelIndex = i;
				DMA_Channel::instances[channelIndex] = this;
				rv = true;
				break;
			}
		}
	}
	return rv;
}

void DMA_Channel::release() {
	if (channel) {
		// diable transfers
		stop();
		// disable any triggers in the ILC
		R_ICU->DELSR[channelIndex] = 0;
		// detach any interrupts so the ILC doesn't hang
		detachInterrupt();
		// release from the pool
		instances[channelIndex] = NULL;
		eventLinkIndex = -1;
		channel = NULL;
	}
}

void DMA_Channel::internalHandler() {
	R_ICU->IELSR[eventLinkIndex] &= ~(R_ICU_IELSR_IR_Msk);
	if (channel) {
		// Clear Interrupt Flag in DMAC
		channel->DMSTS = 0x00;
		if (isrCallback) {
			isrCallback();
		}
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
			R_BSP_IrqDisable((IRQn_Type) eventLinkIndex);
			R_BSP_IrqStatusClear((IRQn_Type) eventLinkIndex);
			NVIC_SetPriority((IRQn_Type) eventLinkIndex, 12);
			R_BSP_IrqEnable((IRQn_Type) eventLinkIndex);
		}
		interrupts();
	}
}

void DMA_Channel::attachInterrupt(void (*isr)()) {
	if (channel) {
		// if we don't already have an interrupt
		if (eventLinkIndex < 0) {
			// Find the event link for our channel
			for (uint8_t i = 0; i < 32; i++) {
				volatile uint32_t val = R_ICU->IELSR[i];
				// RTC_CUP event code is 0x28
				if ((val & 0xFF) == (0x11ul + channelIndex)) {
					eventLinkIndex = i;
					break;
				}
			}
			//If the interrupt isn't in the ILC yet
			if (eventLinkIndex < 0) {
				startInterrupt();
			}
		}

		channel->DMINT = 0x10;
		isrCallback = isr;
	}
}

void DMA_Channel::detachInterrupt() {
	// turns off interrupt but keeps handler registered and eventLinkIndex ready
	if (channel && (eventLinkIndex >= 0)) {
		channel->DMINT = 0;
	}
}

void DMA_Channel::setTriggerSource(uint8_t source) {
	if (channel) {
		// if the channel is on we need to stop and restart it
		bool restart = false;
		if (channel->DMCNT & 1) {
			stop();
			restart = true;
		}
		// diable transfers
		stop();
		if (source) {
			R_ICU->DELSR[channelIndex] = source;
			channel->DMTMD |= 1;
		} else {
			// set as software Trigger
			channel->DMTMD &= ~1;
		}
		// enable transfer if it was running to begin with
		if (restart) {
			start();
		}
	}
}

void DMA_Channel::requestTransfer() {
	if (channel) {
		channel->DMREQ = 0x01;
	}
}

void DMA_Channel::config(DMA_Settings &aSettings) {
	if (channel) {
		settings = aSettings;
		// diable transfers
		stop();
		//set Address Mode Register
		channel->DMAMD = (settings.sourceUpdateMode << R_DMAC0_DMAMD_SM_Pos)
				| (settings.destinationUpdateMode << R_DMAC0_DMAMD_DM_Pos);
		// set transfer Mode
		channel->DMTMD = (settings.mode << R_DMAC0_DMTMD_MD_Pos)
				| (settings.repeatAreaSelection << R_DMAC0_DMTMD_DTS_Pos)
				| (settings.unitSize << R_DMAC0_DMTMD_SZ_Pos);
		// set source and destination address
		channel->DMSAR = settings.sourceAddress;
		channel->DMDAR = settings.destinationAddress;
		// set repeat size and transfer counter and block count
		switch (settings.mode) {
		case NORMAL:
			channel->DMCRA = settings.transferCount;
			channel->DMCRB = 0;
			break;
		case REPEAT:
			channel->DMCRA = (settings.groupSize << 16) | settings.groupSize;
			channel->DMCRB = settings.transferCount;
			break;
		case BLOCK:
			channel->DMCRA = (settings.groupSize << 16) | settings.groupSize;
			channel->DMCRB = settings.transferCount;
			break;
		}
		// set offset register
		channel->DMOFR = settings.addressOffset;
		
		setTriggerSource(settings.triggerSource);

		// enable DMAC controller
		R_DMA->DMAST = 1;
	}
}

bool DMA_Channel::hasChannel() {
	return channel != NULL;
}

void DMA_Channel::stop() {
	if (channel) {
		channel->DMCNT = 0;
	}
}

void DMA_Channel::start() {
	if (channel) {
		channel->DMCNT = 1;
	}
}

void DMA_Channel::resetSourceAddress() {
	if (channel) {
		channel->DMSAR = settings.sourceAddress;
	}
}

void DMA_Channel::resetDestinationAddress() {
	if (channel) {
		channel->DMDAR = settings.destinationAddress;
	}
}

void DMA_Channel::resetCounter() {
	if (channel) {
		switch (settings.mode) {
		case NORMAL:
			channel->DMCRA = settings.transferCount;
			channel->DMCRB = 0;
			break;
		case REPEAT:
			channel->DMCRB = settings.transferCount;
			break;
		case BLOCK:
			channel->DMCRB = settings.transferCount;
			break;
		}
	}
}
