/*

 R4_DMA.h  --  Use DMA controller on UNO-R4 boards.
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

#ifndef R4_DMA_H
#define R4_DMA_H

#include "Arduino.h"
#include "IRQManager.h"

// DMA transfer unit size
typedef enum {
	SZ_8_BIT, 
	SZ_16_BIT, 
	SZ_32_BIT
} SZ_Option;

// DMA Repeat Area Select
typedef enum {
	REPEAT_DESTINATION, 
	REPEAT_SOURCE, 
	NO_REPEAT
} DTS_Option;

// DMA Transfer Mode Selection
typedef enum {
	NORMAL, 
	REPEAT, 
	BLOCK
} MD_Option;

// DMA Source Address Update Mode
typedef enum {
	SOURCE_FIXED, 
	SOURCE_OFFSET, 
	SOURCE_INCREMENT, 
	SOURCE_DECREMENT
} SM_Option;

// DMA Destination Address Update Mode
typedef enum {
	DEST_FIXED, 
	DEST_OFFSET, 
	DEST_INCREMENT, 
	DEST_DECREMENT
} DM_Option;

struct DMA_Settings {

	MD_Option mode;
	SZ_Option unitSize;
	DTS_Option repeatAreaSelection;
	SM_Option sourceUpdateMode;
	DM_Option destUpdateMode;

	uint32_t sourceAddress;
	uint32_t destAddress;
	uint32_t addressOffset;

	uint16_t transferCount;
	uint16_t transferSize;
};

class DMA_Channel {
	
	uint8_t channelIndex;
	DMA_Settings *settings;
	int eventLinkIndex = -1;
	void (*isrCallback)();
	void startInterrupt();
	friend void dtiHandler0();
	friend void dtiHandler1();
	friend void dtiHandler2();
	friend void dtiHandler3();
	static DMA_Channel channels[4];

public:
	R_DMAC0_Type *channel;
	DMA_Channel(uint8_t aChannel);
	void start(DMA_Settings *aSettings);
	void requestTransfer();
	void attachTransferEndInterrupt(void (*isr)());
	void internalHandler();
	void setTriggerSource(uint8_t source);
	static DMA_Channel* getChannel();
};

extern DMA_Channel DMA0;
extern DMA_Channel DMA1;
extern DMA_Channel DMA2;
extern DMA_Channel DMA3;

#endif  //R4_DMA_H
