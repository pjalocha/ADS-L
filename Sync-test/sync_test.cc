#include <stdio.h>
#include <stdint.h>

static int Count1s(uint32_t Word) { return __builtin_popcount(Word); }

int SyncByteTest(uint16_t SYNC, uint16_t Preamble=0x5555)
{ uint32_t Signal = Preamble; Signal<<=16; Signal|=SYNC;
  uint8_t RxSYNC = SYNC>>8;
  printf("Preamble:%04X, SYNC:%04X RxSYNC:%02X\n", Preamble, SYNC, RxSYNC);
  for(int Ofs=0; Ofs<=24; Ofs++)
  { uint8_t RxPipe = Signal>>(24-Ofs);
    int Correl=Count1s(RxPipe^RxSYNC);
    Correl = 4-Correl;
    printf("%2d: %+2d\n", Ofs, Correl); }
  return 0; }

int SyncTwoByteTest(uint16_t SYNC, uint16_t Preamble=0x5555)
{ uint32_t Signal = Preamble; Signal<<=16; Signal|=SYNC;
  printf("Preamble:%04X, SYNC:%04X\n", Preamble, SYNC);
  for(int Ofs=0; Ofs<=16; Ofs++)
  { uint16_t RxPipe = Signal>>(16-Ofs);
    int Correl=Count1s(RxPipe^SYNC);
    Correl = 8-Correl;
    printf("%2d: %+2d\n", Ofs, Correl); }
  return 0; }

int main(int argc, char *argv[])
{ SyncByteTest(0x2DD4, 0x5555);
  SyncByteTest(0x2DD4, 0xAAAA);
  SyncByteTest(0x724B, 0x5555);
  SyncByteTest(0x724B, 0xAAAA);

  SyncTwoByteTest(0x2DD4, 0x5555);
  SyncTwoByteTest(0x2DD4, 0xAAAA);
  SyncTwoByteTest(0x724B, 0x5555);
  SyncTwoByteTest(0x724B, 0xAAAA);

  return 0; }

