# R4_DMA

### A library is for using the DMA controller on Arduino UNO-R4 Minima and UNO-R4 Wifi Boards. 
___
<br><br>
___

## Arduino UNO-R4 DMA Controller Library



The new Arduino UNO-R4 has a powerful new Renesas RA4M1 processor chip.  One of the new features of this chip is a Direct Memory Access Controller (DMAC).  The DMAC allows transfer of bytes between memory locations without CPU involvement.  That means that bytes move from one location to another without the CPU having to run any code.  

This means that an interrupt handler that only copies a value from a register can be implemented via the DMAC with no interrupt overhead.  There’s no state to save, no handler to call, and no stack to restore.  This means the transfer is ultra fast and doesn’t interfere with the timing of any other processes. 

The DMAC on the RA4M1 has four channels, so you can set up four of these transfers at once.  Each channel has a source address and a destination address.  These addresses can be anywhere in memory including any of the peripheral registers or anywhere in SRAM.  When the channel is activated the contents at the source location are copied to the destination.  The size of the transfer can be set to 8, 16 or 32 bits depending on the size of the data you want to transfer.

With each transfer you can set the source address, the destination address, or both to be incremented, decremented or offset.  When incremented or decremented the address simply moves by the size of the transfer.  For instance if you are transferring 32 bit data and have the source set to increment then the source address will get larger by 4 with each transfer.  If the data you want to move is not contiguous, you can set the offset to a 24 bit signed value which will be added to the address each time.  

The DMAC can be activated by a software trigger, but it’s most useful when triggered by one of the event links.  Not all peripherals can trigger the DMAC, but many of them can.  See table 13.4 in the “Renesas RA4M1 Group User’s Manual” for a list of event codes that can be used as DMAC triggers.  

### There are three modes of operation:

* ### NORMAL:
>There is a single transfer with each activation.  You can set a total number of transfers and once that number has been sent the channel turns off and does not activate again.  The DTI interrupt can be triggered at this point.  

* ### REPEAT:
>There is a single transfer with each activation.  You set either the source or destination as the repeat zone and set a repeat size.  Once that number of transfers have happened the address of the repeat zone will be set back to the start and the repeat counter will be reset.  The DTI interrupt can be triggered at this point.  This mode is useful when copying from a single source into a buffer.  

* ### BLOCK:
>Each activation results in the transfer of an entire block of data.  The block size can be up to 1024 bytes.  Both the source and destination addresses need to be updating for this to make sense or it will either just copy the same data to the entire block or copy an entire block one byte at a time to the same location.  You can set a number of blocks to be transferred and once this number of transfers takes place the DTI interrupt can be triggered.  

___

## struct DMA_Settings

The library exposes a struct type DMA_Settings.  The members of this struct and their options are as follows:

* **`mode`** – select any member of the MD_Option enum for the transfer mode<br>
	* **NORMAL**<br>
	* **REPEAT**<br>
	* **BLOCK**<br>

* **`unitSize`** – select any member of the SZ_Option enum for the transfer size<br>
	* **SZ_8_BIT**<br>
	* **SZ_16_BIT**<br>
	* **SZ_32_BIT**<br>

* **`repeatAreaSelection`** – select any member of the DTS_Option enum<br>
	* **REPEAT_DESTINATION**<br>
	* **REPEAT_SOURCE**<br>
	* **NO_REPEAT**<br>

* **`sourceUpdateMode`** – select any member of the SMDM_Option enum<br>
	* **FIXED**<br>
	* **OFFSET**<br>
	* **INCREMENT**<br>
	* **DECREMENT**<br>

* **`destinationUpdateMode`** - select any member of the SMDM_Option enum<br>
	* **FIXED**<br>
	* **OFFSET**<br>
	* **INCREMENT**<br>
	* **DECREMENT**<br>

* **`sourceAddress`** – `(void*)` a pointer to the start of the source area 

* **`destinationAddress`** – `(void*)` a pointer to the start of the destination area 

* **`addressOffset`** – `(uint32_t)` 24 bit signed value to be used if either SOURCE_OFFSET or DEST_OFFSET are chosen as address modes.

* **`groupSize`** – `(uint16_t)` an integer value from 0 to 1024.  In REPEAT mode this sets the repeat size.  In BLOCK mode this sets the block size.

* **`transferCount`** – `(uint16_t)` an integer value from 0 to 65535.  The number of transfers before the channel stops and the  DTI interrupt is triggered.  In NORMAL mode this will be the number of transfers before the channel stops.  In REPEAT mode this will be the number of repeats before the channel stops.  In block mode it will be the number of blocks before the channel stops.  

* **`triggerSource`** – `(uint8_t)` The event code that should trigger the DMA channel.  Not all events can trigger the DMAC.  See table 13.4 in the “Renesas RA4M1 Group User’s Manual” for a list of event codes.  Set the source as 0 to enable software triggering of the DMAC. 

___

## class DMA_Controller

The DMA_Controller class exposes an interface for controlling the DMAC.  Since there are only four channels, only four instances of the class can be active at any given time.  Method calls on instances that are not connected to a channel will do nothing.  

* **`bool getChannel()`** - connects the instance to one of the four DMAC channels if one is available.  Returns true if a channel is assigned and false if there are no channels available.  Calling this function when a channel is already assigned has no effect.  

* **`void config(DMA_Settings &settings)`** – makes a copy of the settings and sets up all the registers to apply them.  Has a side effect of disabling the channel.  You must call start() after config.

* **`void attachInterrupt(void(*isr)())`** - attaches interrupt handler to be called when DTI interrupt is triggered. 

* **`void detachInterrupt()`** - disables the DTI interrupt but leaves the event linked in the ILC.

* **`void setTriggerSource(uint8_t source)`** – sets one of the event link codes to be the trigger source for the DMAC.  Not all events can trigger the DMAC.  See table 13.4 in the “Renesas RA4M1 Group User’s Manual” for a list of event codes.  Set the source as 0 to enable software triggering of the DMAC. If the channel is running when this function is called it will stop and restart the channel. 

* **`void requestTransfer()`** - if the triger source is set to software trigger (setTriggerSource(0)) then this function will initiate the software trigger. 

* **`void release()`** - Stops the DMA channel, terminates any transfers, turns off any interrupts, disables the trigger, and returns the channel to the pool so another instance can use it.  Any further method calls will have no effect except for getChannel which can be called to request a new channel.  

* **`void stop()`** - stops the DMA channel but retains ownership of the channel. 

* **`void start()`** - starts the channel.  At the next trigger a transfer will take place. 

* **`bool hasChannel()`** - returns true if the instance is currently connected to a DMA channel or false if not.


