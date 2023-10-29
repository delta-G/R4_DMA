#define PRINT_REG(BUCKET, REGISTER) \
  do { \
    uint32_t t = BUCKET->REGISTER; \
    Serial.print(#REGISTER " : 0x"); \
    Serial.println(t, HEX); \
  } while (false)


uint32_t source = 0xABCDEF01;
uint32_t destination = 0;


void setup() {

  Serial.begin(115200);
  while (!Serial)
    ;
  Serial.println("\n\n\nStarting R4_DMA_Test.ino");

  
  setupDMA();
  printRegisters(0);

  Serial.print("Before Transfer : ");
  Serial.print(source);
  Serial.print("  :  ");
  Serial.println(destination);

  requestTransfer();
  delay(10); // we can work on timing later

  Serial.print("After Transfer : ");
  Serial.print(source);
  Serial.print("  :  ");
  Serial.println(destination);
  Serial.println("End Setup");
}

void loop() {
}

void requestTransfer(){
  R_DMAC0->DMREQ = 0x11;
}

void setupDMA(){
  // disable controller
  R_DMA->DMAST = 0;
  // diable transfers
  R_DMAC0->DMCNT = 0;
  // DMA Address Mode Register (DMAMD)
  //(SM[1:0]) (-) (SARA[4:0]) (DM[1:0]) (-) (DARA[4:0])
  R_DMAC0->DMAMD = 0;
  // DMA Transfer Mode Register  (DMTMD)
  //(MD[1:0]) (DTS[1:0]) (--) (SZ[1:0]) (------) (DCTG[1:0])
  // (00 Normal transfer) (10 no repeat block) (--) (10 32 bits) (------) (00 Software)
  R_DMAC0->DMTMD = 0x2200;
  // set source address and destination address
  R_DMAC0->DMSAR = (uint32_t)&source;
  R_DMAC0->DMDAR = (uint32_t)&destination;
  // DMCRA to 0 for free running mode
  R_DMAC0->DMCRA = 0;
  // Block transfer
  R_DMAC0->DMCRB = 0;
  // offset register
  R_DMAC0->DMOFR = 0;
  // interrupts
  R_DMAC0->DMINT = 0;

  // enable transfer
  R_DMAC0->DMCNT = 1;
  // enable DMAC controller
  R_DMA->DMAST = 1;
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