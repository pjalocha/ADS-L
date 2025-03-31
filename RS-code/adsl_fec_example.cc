#include <stdio.h>

#include "adsl_rs.h"

static ADSL_RS RS;

static uint8_t Data[255];                                       // storage for packet data and the parity block

int main(int argc, char *argv[])
{
  int DataLen=50;                                               // choose packet size
  for(int Idx=0; Idx<DataLen; Idx++)                            // fill packet with some data
  { Data[Idx]=Idx; }

  RS.Encode(Data, DataLen);                                     // encode the packet
  printf("Data: ");                                             // print the packet
  for(int Idx=0; Idx<DataLen; Idx++)
    printf("%02X", Data[Idx]);
  printf("\nParity: ");                                         // print the parity block
  for(int Idx=DataLen; Idx<DataLen+RS.ParitySize; Idx++)        // parity is placed immediately after the packet dqta
    printf("%02X", Data[Idx]);

  Data[10] ^= 0x34;                                             // corrupt some of the data bytes
  Data[15] ^= 0xED;

  int Err=RS.Decode(Data, DataLen);                             // decode the packet with the parity block
  printf("\n => %d corrected bytes\n", Err);                    // print number of corrected bytes

  return 0; }
