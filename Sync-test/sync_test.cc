#include <stdio.h>
#include <stdint.h>

// A check/test for SYNC words to be used for ADS-L packets
// in other words a simulation how the receiver SYNC-match-correlator oerceives the incoming preamble and SYNC bits

// Pawel Jalocha, 2025

static int Count1s(uint32_t Word) { return __builtin_popcount(Word); } // count 1s in Word

// Test response of a receiver byte-wide correlator to incoming Preamble followed by the SYNC
int SyncByteTest(uint16_t SYNC, uint16_t Preamble=0x5555)
{ uint32_t Signal = Preamble; Signal<<=16; Signal|=SYNC;                      // pack Preamble and then SYNC into 32-bit word
  uint8_t RxSYNC = SYNC>>8;                                                   // the correlator reference: the first SYNC byte
  printf("Preamble:%04X, SYNC:%04X RxSYNC:%02X\n", Preamble, SYNC, RxSYNC);   // print input conditions
  for(int Ofs=0; Ofs<=24; Ofs++)                                              // slide the corelator over the Signal
  { uint8_t RxPipe = Signal>>(24-Ofs);                                        // correlator shift register
    int Correl=Count1s(RxPipe^RxSYNC);                                        // number of bits different from the SYNC byte
    Correl = 4-Correl;                                                        // convert to +/-4 range where 0 is no correlation
    printf("%2d: %+2d\n", Ofs, Correl); }                                     // print
  return 0; }

// same as above but the correlator is two-bytes wide
int SyncTwoByteTest(uint16_t SYNC, uint16_t Preamble=0x5555)
{ uint32_t Signal = Preamble; Signal<<=16; Signal|=SYNC;
  printf("Preamble:%04X, SYNC:%04X\n", Preamble, SYNC);
  for(int Ofs=0; Ofs<=16; Ofs++)
  { uint16_t RxPipe = Signal>>(16-Ofs);
    int Correl=Count1s(RxPipe^SYNC);
    Correl = 8-Correl;
    printf("%2d: %+2d\n", Ofs, Correl); }
  return 0; }

// ideally we want that the correlation is minimal thus 0 or +/-1
// but at a position where the correlator hits exactly the SYNC

// SYNC vectors under investigation:
// ADS-L on M-Band: 0x724B = 0111 0010 0100 1000  MSB transmitted first
// ADS-L on O-Band: 0x2DD4 = 0010 1101 1101 0100  MSB transmitted first
// Pilot-aware:     0xB42B = 1011 0100 0010 1011  LSB transmitted first and it becomes identical to Pilot-Aware SYNC

// Preambles:
// 0x0505 = 01010101
// 0xAAAA = 10101010

int main(int argc, char *argv[])
{ SyncByteTest(0x2DD4, 0x5555);     // 0x2DD4 is proposed for O-band ADS-L
  SyncByteTest(0x2DD4, 0xAAAA);     // and it is not bad with 1010 preamble
  SyncByteTest(0x724B, 0x5555);     // 0x724B would be what the M-band uses
  SyncByteTest(0x724B, 0xAAAA);     // and it is better with 0101 preamble

  printf("\n");
  SyncTwoByteTest(0x2DD4, 0x5555);
  SyncTwoByteTest(0x2DD4, 0xAAAA);
  SyncTwoByteTest(0x724B, 0x5555);
  SyncTwoByteTest(0x724B, 0xAAAA);

  return 0; }

