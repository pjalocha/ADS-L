#include <stdio.h>
#include <stdint.h>

// Hopping channel map for a device which uses only M-Band: M-Band (both channels)
// Function HopChannel() generates channel number 0..3 and this translates to the frequency
// 0/2 => M-band 868.2MHz, 1/3 => M-Band 868.4MHz
// note that the HopChannel() function is same as for a device using all systems/frequencies
// and so there is a 50% overlap between the two devices.


#include "adsl-hop.h"                                 // ADS-L hopping algorithm

// color control for printing out
#define BOLD       "\033[1m"
#define GREEN      "\033[32m"
#define BOLD_GREEN "\033[1;32m"
#define BLUE_BKG   "\033[44m"
#define RESET      "\033[0m"

const float PktTime[4] = { 5.0, 5.0, 8.5, 1.25 } ;    // [ms] packet time duration for M-Band, M-Band, LDR, HDR

int main(int argc, char *argv[])
{ const int Acfts   = 60;                             // 60 aircrafts
  const int RefAcft = 15;                             // reference aircraft is #15 (at 1500m)

  int32_t Alt[Acfts];                                 // assign altitudes
  for(int Idx=0; Idx<Acfts; Idx++)
  { Alt[Idx]=Idx*100; }

  uint8_t Chan[Acfts];                                // actual transmission channel of given aircraft
  int     ChanMiss[Acfts];                            // count how many times the aircraft misses the transmission channel
  uint8_t ChanChange[Acfts];                          // count how many times give aircraft changes the transmission channel
  for(int Idx=0; Idx<Acfts; Idx++)
  { Chan[Idx]=0; ChanChange[Idx]=0; ChanMiss[Idx]=0; }
  int   PktOccup [4] = { 0, 0, 0, 0 };                // occupancy (number of packets) per given channel per second
  float TimeOccup[4] = { 0, 0, 0, 0 };                // occupancy (number or milliseconds) per given channel per second

  for(uint8_t Sec=0; Sec<60; Sec++)
  { int   SlotPktOccup [4] = { 0, 0, 0, 0 };          // [pkt]
    float SlotTimeOccup[4] = { 0, 0, 0, 0 };          // [ms]
    for(int Idx=0; Idx<Acfts; Idx++)
    { uint8_t NewChan = HopChannel(Sec%60, Alt[Idx]);
      NewChan&=1;                                         //
      if(Sec>0 && NewChan!=Chan[Idx]) ChanChange[Idx]+=1; // count channel change
      Chan[Idx]=NewChan;
      SlotTimeOccup[NewChan]+=PktTime[NewChan];
      SlotPktOccup[NewChan]+=1;
      TimeOccup[NewChan]+=PktTime[NewChan];
      PktOccup[NewChan]+=1; }
    printf("%3d:", Sec);
    for(int Idx=0; Idx<Acfts; Idx++)
    { char Mark=' '; if(Idx==RefAcft) Mark='|';
      if(Chan[Idx]==Chan[RefAcft]) Mark='*';
                              else ChanMiss[Idx]+=1;
      Mark = '0'+Chan[Idx];
      if(Idx==RefAcft) printf(BLUE_BKG);
      printf("%c", Mark);
      if(Idx==RefAcft) printf(RESET);
    }
    printf(" %d", Chan[RefAcft]);
    printf(" [%2d %2d %2d+%2d]pkt", SlotPktOccup[0], SlotPktOccup[1], SlotPktOccup[2], SlotPktOccup[3]);
    printf(" [%4.1f %4.1f %4.1f+%4.1f]ms", SlotTimeOccup[0], SlotTimeOccup[1], SlotTimeOccup[2], SlotTimeOccup[3]);
    printf("\n");
  }
  printf("PktOccup [4] = %4d %4d %4d %4d [pkt/min]\n", PktOccup[0], PktOccup[1], PktOccup[2], PktOccup[3]);
  printf("TimeOccup[4] = %4.0f %4.0f %4.0f+%4.0f [ms/min]\n", TimeOccup[0], TimeOccup[1], TimeOccup[2], TimeOccup[3]);

  printf("ChanChange[%d] // number of time radio channel changed", Acfts);
  for(int Idx=0; Idx<Acfts; Idx++)
  { if(Idx%15==0) printf("\n");
    printf(" %3d", ChanChange[Idx]); }
  printf("\n");

  printf("ChanMiss[%d] // number of times the reception was missed due a different channel", Acfts);
  for(int Idx=0; Idx<Acfts; Idx++)
  { if(Idx%15==0) printf("\n");
    printf(" ");
    if(Idx==RefAcft) printf(BLUE_BKG);
    printf("%3d", ChanMiss[Idx]);
    if(Idx==RefAcft) printf(RESET);
  }
  printf("\n");

  return 0; }

