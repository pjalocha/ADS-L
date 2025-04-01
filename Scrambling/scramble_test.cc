#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <math.h>

// ==============================================================================================
// XXTEA encryption/decryption

static uint32_t XXTEA_MX(uint8_t E, uint32_t Y, uint32_t Z, uint8_t P, uint32_t Sum, const uint32_t Key[4])
{ return ((((Z>>5) ^ (Y<<2)) + ((Y>>3) ^ (Z<<4))) ^ ((Sum^Y) + (Key[(P&3)^E] ^ Z))); }

static uint32_t XXTEA_MX_KEY0(uint32_t Y, uint32_t Z, uint8_t P, uint32_t Sum)
{ return ((((Z>>5) ^ (Y<<2)) + ((Y>>3) ^ (Z<<4))) ^ ((Sum^Y) + Z)); }

void XXTEA_Encrypt(uint32_t *Data, uint8_t Words, const uint32_t Key[4], uint8_t Loops)
{ const uint32_t Delta = 0x9e3779b9;
  uint32_t Sum = 0;
  uint32_t Z = Data[Words-1]; uint32_t Y;
  for( ; Loops; Loops--)
  { Sum += Delta;
    uint8_t E = (Sum>>2)&3;
    for (uint8_t P=0; P<(Words-1); P++)
    { Y = Data[P+1];
      Z = Data[P] += XXTEA_MX(E, Y, Z, P, Sum, Key); }
    Y = Data[0];
    Z = Data[Words-1] += XXTEA_MX(E, Y, Z, Words-1, Sum, Key);
  }
}

void XXTEA_Encrypt_Key0(uint32_t *Data, uint8_t Words, uint8_t Loops)
{ const uint32_t Delta = 0x9e3779b9;
  uint32_t Sum = 0;
  uint32_t Z = Data[Words-1]; uint32_t Y;
  for( ; Loops; Loops--)
  { Sum += Delta;
    for (uint8_t P=0; P<(Words-1); P++)
    { Y = Data[P+1];
      Z = Data[P] += XXTEA_MX_KEY0(Y, Z, P, Sum); }
    Y = Data[0];
    Z = Data[Words-1] += XXTEA_MX_KEY0(Y, Z, Words-1, Sum);
  }
}

void XXTEA_Decrypt(uint32_t *Data, uint8_t Words, const uint32_t Key[4], uint8_t Loops)
{ const uint32_t Delta = 0x9e3779b9;
  uint32_t Sum = Loops*Delta;
  uint32_t Y = Data[0]; uint32_t Z;
  for( ; Loops; Loops--)
  { uint8_t E = (Sum>>2)&3;
    for (uint8_t P=Words-1; P; P--)
    { Z = Data[P-1];
      Y = Data[P] -= XXTEA_MX(E, Y, Z, P, Sum, Key); }
    Z = Data[Words-1];
    Y = Data[0] -= XXTEA_MX(E, Y, Z, 0, Sum, Key);
    Sum -= Delta;
  }
}

void XXTEA_Decrypt_Key0(uint32_t *Data, uint8_t Words, uint8_t Loops)
{ const uint32_t Delta = 0x9e3779b9;
  uint32_t Sum = Loops*Delta;
  uint32_t Y = Data[0]; uint32_t Z;
  for( ; Loops; Loops--)
  { for (uint8_t P=Words-1; P; P--)
    { Z = Data[P-1];
      Y = Data[P] -= XXTEA_MX_KEY0(Y, Z, P, Sum); }
    Z = Data[Words-1];
    Y = Data[0] -= XXTEA_MX_KEY0(Y, Z, 0, Sum);
    Sum -= Delta;
  }
}

// ==============================================================================================

int Count1s(uint32_t Word) { return __builtin_popcountl(Word); }

// ==============================================================================================

const int PacketWords = 5;                // [32-bt words]
const int PacketBytes = PacketWords*4;    // [8-bit bytes]
const int PacketBits  = PacketWords*32;

const uint32_t Key[4] = { 0, 0, 0, 0 };

uint32_t Packet[PacketWords] = { 0x01234567, 0x89ABCDEF, 0xFEDCBA98, 0x76543210, 0x12345678 } ;   // some initial "random" data
uint32_t Scrambled[PacketWords];
uint32_t Descrambled[PacketWords];

void Copy(      uint32_t *Dst, const uint32_t *Src) { memcpy(Dst, Src, PacketWords*sizeof(uint32_t)); }
bool Same(const uint32_t *Dst, const uint32_t *Src) { return memcmp(Dst, Src, PacketWords*sizeof(uint32_t))==0; }

int DiffBits(const uint32_t *Dst, const uint32_t *Src)
{ int Count=0;
  for(int Idx=0; Idx<PacketWords; Idx++)
  { Count+=Count1s(Src[Idx]^Dst[Idx]); }
  return Count; }

void Print(uint32_t *Packet)
{ for(int Idx=0; Idx<PacketWords; Idx++)
    printf(" %08X", Packet[Idx]);
}

void FlipBit(uint32_t *Packet, int Bit)
{ int Byte = Bit>>5;
  Bit&=31;
  uint32_t Mask=1; Mask<<=Bit;
  Packet[Byte]^=Mask; }

const int Tests = 4000000;
const int TeaLoops = 5;

int main(int argc, char *argv[])
{

  printf("%d tests, %d XXTEA loops\n", Tests, TeaLoops);

  int BadCount=0;
  for(int Test=0; Test<Tests; Test++)
  { Copy(Scrambled, Packet);
    // XXTEA_Encrypt(Scrambled, PacketWords, Key, TeaLoops);
    XXTEA_Encrypt_Key0(Scrambled, PacketWords, TeaLoops);
    Copy(Descrambled, Scrambled);
    XXTEA_Decrypt_Key0(Descrambled, PacketWords, TeaLoops);
    bool OK = Same(Descrambled, Packet);
    if(!OK)
    { Print(Packet); printf(" =>"); Print(Scrambled); printf(" => %s\n", OK?"OK":"Bad !");
      BadCount++; }
    Copy(Packet, Scrambled); Packet[0]^=rand(); }
  printf("Reversibility test: %d bad over %d packets\n", BadCount, Tests);

  uint64_t Sum=0;
  uint64_t SumSqr=0;
  int MinBits=PacketBits;
  int MaxBits=0;
  for(int Test=0; Test<Tests; Test++)
  { Copy(Scrambled, Packet);
    int Bit = Packet[0]%PacketBits;                                  // choose bit to flip
    XXTEA_Encrypt_Key0(Scrambled, PacketWords, TeaLoops);
    Copy(Descrambled, Scrambled);
    FlipBit(Descrambled, Bit);                                        // flip bit
    XXTEA_Decrypt_Key0(Descrambled, PacketWords, TeaLoops);
    int Bits = DiffBits(Descrambled, Packet);
    // Print(Packet); printf(" =>"); Print(Scrambled); printf(" => %3d\n", Bits);
    Sum += Bits;
    SumSqr += Bits*Bits;
    if(Bits>MaxBits) MaxBits=Bits;
    if(Bits<MinBits) MinBits=Bits;
    Copy(Packet, Scrambled); }
  double Aver = (double)Sum/Tests;
  double RMS  = sqrt(((double)SumSqr-Aver*Aver*Tests)/Tests);
  printf("Single bit flip test, number of changed bits: %2d..<%4.1f:%3.1f>..%3d\n", MinBits, Aver, RMS, MaxBits);

  return 0; }

