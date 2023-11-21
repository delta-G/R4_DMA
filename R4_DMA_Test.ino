#define PRINT_REG(BUCKET, REGISTER) \
  do { \
    uint32_t t = BUCKET->REGISTER; \
    Serial.print(#REGISTER " : 0x"); \
    Serial.println(t, HEX); \
  } while (false)

#include "EventLinkInterrupt.h"
#include "R4_DMA.h"

const uint8_t transferSize = 5;

uint32_t source[transferSize + 15] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20 };
uint32_t destination[transferSize + 15] = { 0 };

uint8_t buttonPin = 7;
uint8_t oldButtonState = HIGH;

int dtiEventLinkIndex;
DMA_Settings settings;

void dtiHandler(){
  resetEventLink(dtiEventLinkIndex);
  // Clear Interrupt Flag in DMAC
  R_DMAC0->DMSTS = 0x00;
  // reset Source Address
  R_DMAC0->DMSAR = (uint32_t)&source;
  //reset counter
  R_DMAC0->DMCRB = 5;
  // Re-enable DMAC
  R_DMAC0->DMCNT = 1;

  
}

void setup() {

  pinMode(buttonPin, INPUT_PULLUP);
  Serial.begin(115200);
  while (!Serial)
    ;
  Serial.println("\n\n\nStarting R4_DMA_Test.ino");


  setupDMA();
  DMA0.start(&settings);

  Serial.println("End Setup");
}

void loop() {
  uint8_t buttonState = digitalRead(buttonPin);
  if (buttonState != oldButtonState) {
    delay(20);
    buttonState = digitalRead(buttonPin);
    if (buttonState == LOW) {
      doTransfer();
    }
    oldButtonState = buttonState;
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

// void requestTransfer() {
//   R_DMAC0->DMREQ = 0x01;
  
// }

void setupDMA() {

  // // disable controller
  // R_DMA->DMAST = 0;
  // // diable transfers
  // R_DMAC0->DMCNT = 0;
  // DMA Address Mode Register (DMAMD)
  //(SM[1:0]) (-) (SARA[4:0]) (DM[1:0]) (-) (DARA[4:0])
  // R_DMAC0->DMAMD = 0x8080;
  settings.sourceUpdateMode = SOURCE_INCREMENT;
  settings.destUpdateMode = DEST_INCREMENT;
  // DMA Transfer Mode Register  (DMTMD)
  //(MD[1:0]) (DTS[1:0]) (--) (SZ[1:0]) (------) (DCTG[1:0])
  // (01 Repeat transfer) (00 destination repeat block) (--) (10 32 bits) (------) (00 Software)
  // R_DMAC0->DMTMD = 0x8200;
  settings.mode = BLOCK;
  settings.repeatAreaSelection = REPEAT_DESTINATION;
  settings.transferSize = SZ_32_BIT;
  settings.triggerSource = SOFTWARE;
  // set source address and destination address
  // R_DMAC0->DMSAR = (uint32_t)&source;
  // R_DMAC0->DMDAR = (uint32_t)destination + 8;
  settings.sourceAddress = (uint32_t)&source;
  settings.destAddress = (uint32_t)destination + 8;
  // DMCRA to repeat size 3 
  // R_DMAC0->DMCRA = 0x00030003;
  settings.blockSize = 3;
  // Block transfer
  // R_DMAC0->DMCRB = 5;
  settings.transferCount = 5;
  // offset register
  // R_DMAC0->DMOFR = 0;
  // interrupts
  // R_DMAC0->DMINT = 0x10;

  // dtiEventLinkIndex = attachEventLinkInterrupt(0x11, dtiHandler);
  
  // // enable transfer
  // R_DMAC0->DMCNT = 1;
  // // enable DMAC controller
  // R_DMA->DMAST = 1;
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