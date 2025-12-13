// Attempt to produce an altitude-dependend sequence to hop across 2 channels.

#include <stdio.h>
#include <stdint.h>

#include "adsl-hop.h"

// color control for printing out
#define BOLD       "\033[1m"
#define GREEN      "\033[32m"
#define BOLD_GREEN "\033[1;32m"
#define BLUE_BKG   "\033[44m"
#define RESET      "\033[0m"

int main(int argc, char *argv[])
{ const int Acfts   = 60;                             // 60 aircrafts from 0 to 6000m
  const int RefAcft = 10;                             // reference aircraft at 1000m

  int32_t Alt[Acfts];
  for(int Idx=0; Idx<Acfts; Idx++)
  { Alt[Idx]=150+Idx*100; }

  uint8_t Chan[Acfts];
  int     ChanMiss[Acfts];
  uint8_t ChanChange[Acfts];
  for(int Idx=0; Idx<Acfts; Idx++)
  { Chan[Idx]=0; ChanChange[Idx]=0; ChanMiss[Idx]=0; }

  int Occup[3] = { 0, 0, 0 };
  for(uint8_t Sec=0; Sec<60; Sec++)
  { int SlotOccup[3] = { 0, 0, 0 };
    for(int Idx=0; Idx<Acfts; Idx++)
    { uint8_t NewChan = MBandChan(Sec%60, Alt[Idx]);
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

  printf("ChanChange[%d] // number of time radio channel changed", Acfts);
  for(int Idx=0; Idx<Acfts; Idx++)
  { if(Idx%15==0) printf("\n");
    printf(" %3d", ChanChange[Idx]); }
  printf("\n");

  printf("ChanMiss[%d] // number of times the reception was missded due a different channel", Acfts);
  for(int Idx=0; Idx<Acfts; Idx++)
  { if(Idx%15==0) printf("\n");
    printf(" ");
    if(Idx==RefAcft) printf(BLUE_BKG);
    printf("%3d", ChanMiss[Idx]);
    if(Idx==RefAcft) printf(RESET);
  }
  printf("\n");

  return 0; }

