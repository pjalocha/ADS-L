#include <stdio.h>

#include "uat_rs.h"

static int8_t Read_Dec1(char Digit)                   // convert single digit into an integer
{ if(Digit<'0') return -1;                            // return -1 if not a decimal digit
  if(Digit>'9') return -1;
  return Digit-'0'; }

int8_t Read_Hex1(char Digit)
{ int8_t Val=Read_Dec1(Digit); if(Val>=0) return Val;
  if( (Digit>='A') && (Digit<='F') ) return Digit-'A'+10;
  if( (Digit>='a') && (Digit<='f') ) return Digit-'a'+10;
  return -1; }

static int ReadHex(uint8_t *Data, const char *Inp)
{ int Len=0;
  for( ; ; )
  { int8_t H=Read_Hex1(Inp[0]); if(H<0) break;
    int8_t L=Read_Hex1(Inp[1]); if(L<0) break;
    Data[Len++] = (H<<4) | L; Inp+=2; }
  return Len; }

static UAT_RS RS;

static uint8_t Data[255];
// static uint8_t Parity[20];

int main(int argc, char *argv[])
{
  ReadHex(Data, "007E6987D2D74238C1453855F89980C7524F");      // read-in an UAT packet
  uint8_t *Parity = Data + RS.ShortPacket;

  RS.EncodeShort(Data, Parity);                               // encode: generate the parity block
  Data[0]+=1;                                                 // corrupt one data byte
  Parity[1]+=2;                                               // corrupt one parity byte
  int Err=RS.DecodeShort(Data);                               // decode the packet+parity with two bytes corrupted

  printf("Data: ");
  for(int Idx=0; Idx<18; Idx++) printf("%02X", Data[Idx]);
  printf(" + Parity: ");
  for(int Idx=0; Idx<12; Idx++) printf("%02X", Parity[Idx]);
  printf(" => %d bytes corrected\n", Err);

  return 0; }

