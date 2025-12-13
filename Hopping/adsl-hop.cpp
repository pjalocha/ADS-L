#include "adsl-hop.h"

static uint8_t BitRev(uint8_t Byte)                   // reverse bits in a byte
{ Byte = ((Byte&0xF0)>>4) | ((Byte&0x0F)<<4);
  Byte = ((Byte&0xCC)>>2) | ((Byte&0x33)<<2);
  Byte = ((Byte&0xAA)>>1) | ((Byte&0x55)<<1);
  return Byte; }

static uint8_t Scramble(uint8_t Sec)                  // [0..59] scramble a second
{ // Sec^=0x13;
  // Sec+=Sec<<2;
  Sec=BitRev(Sec);
  // if(Sec<60) return Sec;
  return Sec%60; }

uint8_t MBandChan(uint8_t Sec, int32_t Alt)           // decide on the channel based on Second and Altitude
{ if(Alt<0) Alt=0;
  if(Sec>=60) Sec-=60;
  Sec=59-Sec;
  uint16_t AltBand = Alt/100;                         // [100m] altitude band to select hopping pattern
  uint8_t HopPhase = AltBand%60;                      // [0..59] slot phase depends on the altitude band
  uint8_t ScrSec = Scramble(Sec);                     // [0..59] scrambled second
  uint8_t ChSec = ScrSec+HopPhase; if(ChSec>=60) ChSec-=60;
  // uint8_t MinQ = ChSec/15;                            // [0..3] quarter of the minute
  // uint8_t Chan = MinQ;                                // [0..2]
  // if(Chan>2) Chan=(ChSec/5)%3;
  // Chan += Sec*2;
  // return Chan%3; }                                    // [0..2]
  uint8_t Chan = ChSec/25;
  if(Chan>1) Chan=(ChSec-50)/5;
  return Chan; }

uint8_t HopBand(uint8_t Sec, int32_t Alt)             // decide on the band based on Second and Altitude
{ if(Alt<0) Alt=0;
  if(Sec>=60) Sec-=60;
  uint16_t AltBand = Alt/100;                         // [100m] altitude band to select hopping pattern
  uint8_t HopPhase = AltBand%60;                      // [0..59] slot phase depends on the altitude band
  uint8_t ScrSec = Scramble(Sec);                     // [0..59] scrambled second
  uint8_t ChSec = ScrSec+HopPhase; if(ChSec>=60) ChSec-=60;
  uint8_t MinQ = ChSec/15;                            // [0..3] quarter of the minute
  uint8_t Chan = MinQ;                                // [0..2]
  if(Chan>2) Chan=(ChSec/5)%3;
  Chan += Sec*2;
  return Chan%3; }                                    // [0..2]

