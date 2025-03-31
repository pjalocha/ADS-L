#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "adsl_rs.h"

static ADSL_RS RS;

static uint8_t TxData[255];
static uint8_t RxData[255];

int main(int argc, char *argv[])
{
  srand(time(0));

  float BER = 0.03;                                           // byte-error-rate for the test

  int Tests=10000;                                            // number of test packets
  int DecodeErrors=0;
  int DataErrors=0;
  int CorrBytes=0;

  int Verbose=0;

  for(int Test=0; Test<Tests; Test++)                         // make given number of test packets
  { int DataLen = 20+rand()%199;                              // chose size on random

    for(int Idx=0; Idx<DataLen; Idx++)                        // fill packet with random bytes
    { TxData[Idx]=rand(); }
    RS.Encode(TxData, DataLen);                               // encode the packet: produce the parity block

    if(Verbose)
    { printf("TxData[%d] ", DataLen);
      for(int Idx=0; Idx<DataLen; Idx++)
        printf("%02X", TxData[Idx]);
      printf("\nParity[%d] ", RS.ParitySize);
      for(int Idx=DataLen; Idx<DataLen+RS.ParitySize; Idx++)
        printf("%02X", TxData[Idx]);
      printf("\n"); }

    for(int Idx=0; Idx<DataLen+RS.ParitySize; Idx++)          // corrupt some of the bytes
    { float Rand = (float)(rand()%0xFFFF)/0xFFFF;
      bool Err = Rand<BER;                                    // according to the chosen error rate
      RxData[Idx]=TxData[Idx];
      if(Err) RxData[Idx]+=(uint8_t)rand(); }

    if(Verbose)
    { printf("RxData[%d] ", DataLen);
      for(int Idx=0; Idx<DataLen; Idx++)
        printf("%02X", RxData[Idx]);
      printf("\nParity[%d] ", RS.ParitySize);
      for(int Idx=DataLen; Idx<DataLen+RS.ParitySize; Idx++)
        printf("%02X", RxData[Idx]);
      printf("\n"); }

    int Err=RS.Decode(RxData, DataLen);                      // decode the packet and parity block: possibly corrupted
    if(Verbose) printf("Decode: %d\n", Err);                 // print number of byte in error detected

    if(Err<0) DecodeErrors++;                                // count packets which could not be corrected
    else
    { CorrBytes+=Err;
      if(memcmp(RxData, TxData, DataLen+RS.ParitySize)!=0) DataErrors++; }
  }

  printf("%d tests => errors: decode:%d, data:%d => corrected bytes: %d\n",
              Tests, DecodeErrors, DataErrors, CorrBytes);

  return 0; }
