#include "Arduino_LED_Matrix.h"
#include "frames.h"
#include "R4_DMA.h"


ArduinoLEDMatrix matrix;  // Create an instance of the ArduinoLEDMatrix class
DMA_Channel dma;          // Create an instance of the DMA_Channel class

extern uint8_t framebuffer[12];  // extern reference to the framebuffer in Arduino_LED_Matrix.h

//  Interrupt handler to be called after the last frame
void xferEndHandler(){
  dma.resetSourceAddress();
  dma.resetCounter();
  dma.start();
}

void setup() {
  // Start AGT1 at 1000ms interval
  setupAGT1();
  matrix.begin();

  // Create an instance of DMA_Settings and set the options
  DMA_Settings settings;
  settings.mode = BLOCK;
  settings.unitSize = SZ_32_BIT;
  settings.repeatAreaSelection = REPEAT_DESTINATION;
  settings.sourceUpdateMode = INCREMENT;
  settings.destinationUpdateMode = INCREMENT;
  settings.sourceAddress = (uint32_t)frames;
  settings.destinationAddress = (uint32_t)framebuffer;
  settings.addressOffset = 0;
  settings.groupSize = 3;
  settings.transferCount = 7;

  // get a DMA channel
  if(!dma.getChannel()){
    while(1);
  }
  // Configure the channel
  dma.config(settings);
  // attach the interrupt
  dma.attachInterrupt(xferEndHandler);
  // set the trigger source to the AGT1 underflow (event 0x21)
  dma.setTriggerSource(0x21);
  // Start the DMA channel
  dma.start();

}

void loop() {

  // empty loop
  delay(10);

}


void setupAGT1() {
  // enable the timer in Module Stop Control Register D
  R_MSTP->MSTPCRD &= ~(1 << R_MSTP_MSTPCRD_MSTPD2_Pos);
  //  Make sure timer is stopped while we adjust registers.
  R_AGT1->AGTCR = 0;

  // We're using R_AGT1, but all the positions and bitmasks are defined as R_AGT0
  // set mode register 1
  //(-) (TCK[2:0]) (TEDGPL) (TMOD[2:0])
  //  Use TIMER mode with the LOCO clock (best we can do since Arduino doesn't have crystal for SOSC)
  R_AGT1->AGTMR1 = (4 << R_AGT0_AGTMR1_TCK_Pos);
  // mode register 2
  // (LPM) (----) (CKS[2:0])
  R_AGT1->AGTMR2 = 0;
  // AGT I/O Control Register
  // (TIOGT[1:0]) (TIPF[1:0]) (-) (TOE) (-) (TEDGSEL)
  R_AGT1->AGTIOC = 0;
  // Event Pin Select Register
  // (-----) (EEPS) (--)
  R_AGT1->AGTISR = 0;
  // AGT Compare Match Function Select Register
  // (-) (TOPOLB) (TOEB) (TCMEB) (-) (TOPOLA) (TOEA) (TCMEA)
  R_AGT1->AGTCMSR = 0;
  // AGT Pin Select Register
  // (---) (TIES) (--) (SEL[1:0])
  R_AGT1->AGTIOSEL = 0;
  R_AGT1->AGT = 32768;  // 1000ms

  // // set compare registers to some values
  // R_AGT1->AGTCMA = 21845;
  // R_AGT1->AGTCMB = 10923;
  // //enable compare match
  // R_AGT1->AGTCMSR = 0x11;  // both enable bits

  R_AGT1->AGTCR = 1;
}