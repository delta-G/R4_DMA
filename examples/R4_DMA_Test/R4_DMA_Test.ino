#define PRINT_REG(BUCKET, REGISTER) \
  do { \
    uint32_t t = BUCKET->REGISTER; \
    Serial.print(#REGISTER " : 0x"); \
    Serial.println(t, HEX); \
  } while (false)

#include "R4_DMA.h"

const uint8_t transferSize = 5;

uint32_t source[transferSize + 15] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20 };
uint32_t destination[transferSize + 15] = { 0 };

uint8_t buttonPin = 7;
uint8_t oldButtonState = HIGH;

int dtiEventLinkIndex;
DMA_Settings settings[4];
DMA_Channel* DMA[4];

void xferEndHandler0(){
  // reset Source Address
  DMA[0]->channel->DMSAR = (uint32_t)&source;
  //reset counter
  DMA[0]->channel->DMCRB = 5;
  // Re-enable DMAC
  DMA[0]->channel->DMCNT = 1;  
}
void xferEndHandler1(){
  // reset Source Address
  DMA[1]->channel->DMSAR = (uint32_t)&source + 4;
  //reset counter
  DMA[1]->channel->DMCRB = 5;
  // Re-enable DMAC
  DMA[1]->channel->DMCNT = 1;  
}
void xferEndHandler2(){
  // reset Source Address
  DMA[2]->channel->DMSAR = (uint32_t)&source + 8;
  //reset counter
  DMA[2]->channel->DMCRB = 5;
  // Re-enable DMAC
  DMA[2]->channel->DMCNT = 1;  
}


void setup() {

  pinMode(buttonPin, INPUT_PULLUP);
  Serial.begin(115200);
  while (!Serial)
    ;
  Serial.println("\n\n\nStarting R4_DMA_Test.ino");

  setupAGT1();
  setupDMA();
  DMA[0] = DMA_Channel::getChannel();
  if(!DMA[0]){
    Serial.println("Didn't get DMA Channel 0!");
    while(1);
  }
  DMA[0]->start(&(settings[0]));
  DMA[0]->attachTransferEndInterrupt(xferEndHandler0);  
  DMA[0]->setTriggerSource(0x21); // set for AGT1 underflow

  DMA[1] = DMA_Channel::getChannel();
  if(!DMA[1]){
    Serial.println("Didn't get DMA Channel 1!");
    while(1);
  }
  DMA[1]->start(&(settings[1]));
  DMA[1]->attachTransferEndInterrupt(xferEndHandler1);  
  DMA[1]->setTriggerSource(0x22); // set for AGT1 compare match A

  DMA[2] = DMA_Channel::getChannel();
  if(!DMA[2]){
    Serial.println("Didn't get DMA Channel 1!");
    while(1);
  }
  DMA[2]->start(&(settings[2]));
  DMA[2]->attachTransferEndInterrupt(xferEndHandler2);  
  DMA[2]->setTriggerSource(0x23); // set for AGT1 compare match B


  Serial.println("End Setup");
}

void loop() {
  static uint32_t oldVal = 0;
  if(destination[2] != oldVal){
    oldVal = destination[2];
    printOutput();
  }
}

void doTransfer() {
  // for (int i = 0; i < transferSize + 15; i++) {
  //   source[i]++;
  // }
  printRegisters(0);

  Serial.print("Before Transfer : ");
  printOutput();

  DMA0.requestTransfer();
  delay(10);  // we can work on timing later

  Serial.print("After Transfer : ");
  printOutput();
}

void printOutput() {
  Serial.println();
  for (int i = 0; i < transferSize + 15; i++) {
    Serial.print(" : ");
    Serial.print(source[i]);
  }
  Serial.println();
  for (int i = 0; i < transferSize + 15; i++) {
    Serial.print(" : ");
    Serial.print(destination[i]);
  }
  Serial.println();
}

void setupDMA() {
  settings[0].sourceUpdateMode = SOURCE_INCREMENT;
  settings[0].destUpdateMode = DEST_INCREMENT;
  settings[0].mode = BLOCK;
  settings[0].repeatAreaSelection = REPEAT_DESTINATION;
  settings[0].unitSize = SZ_32_BIT;
  settings[0].sourceAddress = (uint32_t)&source;
  settings[0].destAddress = (uint32_t)destination + 8;
  settings[0].transferSize = 3;
  settings[0].transferCount = 5;

  settings[1].sourceUpdateMode = SOURCE_INCREMENT;
  settings[1].destUpdateMode = DEST_INCREMENT;
  settings[1].mode = BLOCK;
  settings[1].repeatAreaSelection = REPEAT_DESTINATION;
  settings[1].unitSize = SZ_32_BIT;
  settings[1].sourceAddress = (uint32_t)&source + 1;
  settings[1].destAddress = (uint32_t)destination + 32;
  settings[1].transferSize = 3;
  settings[1].transferCount = 5;

  settings[2].sourceUpdateMode = SOURCE_INCREMENT;
  settings[2].destUpdateMode = DEST_INCREMENT;
  settings[2].mode = BLOCK;
  settings[2].repeatAreaSelection = REPEAT_DESTINATION;
  settings[2].unitSize = SZ_32_BIT;
  settings[2].sourceAddress = (uint32_t)&source + 2;
  settings[2].destAddress = (uint32_t)destination + 56;
  settings[2].transferSize = 3;
  settings[2].transferCount = 5;

  // settings[3].sourceUpdateMode = SOURCE_INCREMENT;
  // settings[3].destUpdateMode = DEST_INCREMENT;
  // settings[3].mode = BLOCK;
  // settings[3].repeatAreaSelection = REPEAT_DESTINATION;
  // settings[3].unitSize = SZ_32_BIT;
  // settings[3].sourceAddress = (uint32_t)&source;
  // settings[3].destAddress = (uint32_t)destination + 8;
  // settings[3].transferSize = 3;
  // settings[3].transferCount = 5;
  
}

void setupAGT1(){
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
    R_AGT1->AGT = 32768;   // 1000ms

    // set compare registers to some values
    R_AGT1->AGTCMA = 21845;
    R_AGT1->AGTCMB = 10923;
    //enable compare match
    R_AGT1->AGTCMSR = 0x11;  // both enable bits

    R_AGT1->AGTCR = 1;
}

void printRegisters(uint8_t ch) {
  Serial.print("\nPrinting Registers for channel ");
  Serial.println(ch);

  PRINT_REG(R_DMA, DMAST);
  switch (ch) {
    case 0:
      PRINT_REG(R_DMAC0, DMSAR);
      PRINT_REG(R_DMAC0, DMDAR);
      PRINT_REG(R_DMAC0, DMCRA);
      PRINT_REG(R_DMAC0, DMCRB);
      PRINT_REG(R_DMAC0, DMTMD);
      PRINT_REG(R_DMAC0, DMINT);
      PRINT_REG(R_DMAC0, DMAMD);
      PRINT_REG(R_DMAC0, DMOFR);
      PRINT_REG(R_DMAC0, DMCNT);
      PRINT_REG(R_DMAC0, DMREQ);
      PRINT_REG(R_DMAC0, DMSTS);
      break;
    case 1:
      PRINT_REG(R_DMAC1, DMSAR);
      PRINT_REG(R_DMAC1, DMDAR);
      PRINT_REG(R_DMAC1, DMCRA);
      PRINT_REG(R_DMAC1, DMCRB);
      PRINT_REG(R_DMAC1, DMTMD);
      PRINT_REG(R_DMAC1, DMINT);
      PRINT_REG(R_DMAC1, DMAMD);
      PRINT_REG(R_DMAC1, DMOFR);
      PRINT_REG(R_DMAC1, DMCNT);
      PRINT_REG(R_DMAC1, DMREQ);
      PRINT_REG(R_DMAC1, DMSTS);
      break;
    case 2:
      PRINT_REG(R_DMAC2, DMSAR);
      PRINT_REG(R_DMAC2, DMDAR);
      PRINT_REG(R_DMAC2, DMCRA);
      PRINT_REG(R_DMAC2, DMCRB);
      PRINT_REG(R_DMAC2, DMTMD);
      PRINT_REG(R_DMAC2, DMINT);
      PRINT_REG(R_DMAC2, DMAMD);
      PRINT_REG(R_DMAC2, DMOFR);
      PRINT_REG(R_DMAC2, DMCNT);
      PRINT_REG(R_DMAC2, DMREQ);
      PRINT_REG(R_DMAC2, DMSTS);
      break;
    case 3:
      PRINT_REG(R_DMAC3, DMSAR);
      PRINT_REG(R_DMAC3, DMDAR);
      PRINT_REG(R_DMAC3, DMCRA);
      PRINT_REG(R_DMAC3, DMCRB);
      PRINT_REG(R_DMAC3, DMTMD);
      PRINT_REG(R_DMAC3, DMINT);
      PRINT_REG(R_DMAC3, DMAMD);
      PRINT_REG(R_DMAC3, DMOFR);
      PRINT_REG(R_DMAC3, DMCNT);
      PRINT_REG(R_DMAC3, DMREQ);
      PRINT_REG(R_DMAC3, DMSTS);
      break;
  }
}