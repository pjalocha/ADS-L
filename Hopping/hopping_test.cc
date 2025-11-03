#include <stdio.h>
#include <stdint.h>

static uint8_t BitRev(uint8_t Byte)
{ Byte = ((Byte&0xF0)>>4) | ((Byte&0x0F)<<4);
  Byte = ((Byte&0xCC)>>2) | ((Byte&0x33)<<2);
  Byte = ((Byte&0xAA)>>1) | ((Byte&0x55)<<1);
  return Byte; }

static uint8_t Scramble(uint8_t Sec)
{ Sec=BitRev(Sec);
  // Sec = Sec*13+37;
  // Sec = Sec*29+47;
  // Sec = Sec*73+19;
  return Sec%60; }

static uint8_t HopChan(uint8_t Sec, int32_t Alt)
{ if(Alt<0) Alt=0;
  uint16_t AltBand = Alt/100;                         // [100m] altitude band to select hopping pattern
  uint8_t HopPhase = AltBand%60;                      // [0..59]
  uint8_t ScrSec = Scramble(Sec);                     // [0..59] scrambled second
  uint8_t ChSec = ScrSec+HopPhase; if(ChSec>=60) ChSec-=60;
  uint8_t MinQ = ChSec/15;                            // [0..3] quarter of the minute
  uint8_t Chan = MinQ; // if(Chan>2) Chan=1;             // [0..2]
  if(Chan>2) Chan=(ChSec/5)%3;
  Chan += Sec*2;
  // Chan += Scramble(Sec);
  return Chan%3; }                                    // [0..2]

// color control for printing out
#define BOLD       "\033[1m"
#define GREEN      "\033[32m"
#define BOLD_GREEN "\033[1;32m"
#define BLUE_BKG   "\033[44m"
#define RESET      "\033[0m"

int main(int argc, char *argv[])
{ const int Acfts = 40;
  const int RefAcft = 2;

  int32_t Alt[Acfts];
  for(int Idx=0; Idx<Acfts; Idx++)
  { Alt[Idx]=150+Idx*100; }

  uint8_t Chan[Acfts];
  int     ChanMiss[Acfts];
  uint8_t ChanChange[Acfts];
  for(int Idx=0; Idx<Acfts; Idx++)
  { Chan[Idx]=0; ChanChange[Idx]=0; ChanMiss[Idx]=0; }

  int Occup[3] = { 0, 0, 0 };
  for(uint8_t Sec=0; Sec<120; Sec++)
  { int SlotOccup[3] = { 0, 0, 0 };
    for(int Idx=0; Idx<Acfts; Idx++)
    { uint8_t NewChan = HopChan(Sec%60, Alt[Idx]);
      if(Sec>0 && NewChan!=Chan[Idx]) ChanChange[Idx]+=1;
      Chan[Idx]=NewChan;
      SlotOccup[NewChan]+=1;
      Occup[NewChan]+=1; }
    printf("%3d:", Sec);
    for(int Idx=0; Idx<Acfts; Idx++)
    { char Mark=' '; if(Idx==RefAcft) Mark='|';
      if(Chan[Idx]==Chan[RefAcft]) Mark='*';
                              else ChanMiss[Idx]+=1;
      if(Idx==RefAcft) printf(BLUE_BKG);
      printf("%c", Mark);
      if(Idx==RefAcft) printf(RESET);
    }
    printf(" %d [%2d:%2d:%2d]\n", Chan[RefAcft], SlotOccup[0], SlotOccup[1], SlotOccup[2]); }

  printf("Occup[3] = %d %d %d\n", Occup[0], Occup[1], Occup[2]);

  printf("ChanChange[%d]", Acfts);
  for(int Idx=0; Idx<Acfts; Idx++)
  { if(Idx%15==0) printf("\n");
    printf(" %3d", ChanChange[Idx]); }
  printf("\n");

  printf("ChanMiss[%d]", Acfts);
  for(int Idx=0; Idx<Acfts; Idx++)
  { if(Idx%15==0) printf("\n");
    printf(" %3d", ChanMiss[Idx]); }
  printf("\n");

  return 0; }

