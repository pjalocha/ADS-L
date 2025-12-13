#include <stdio.h>
#include <stdint.h>

#include "adsl-hop.h"

int main(int argc, char *argv[])
{
  for(uint8_t Sec=0; Sec<60; Sec++)
  { printf("%3d: ", Sec);
    for(uint16_t Alt=0; Alt<6000; Alt+=100)
    { uint8_t Band=HopBand(Sec, Alt);
      if(Band) printf("%c", Band==1?'L':'H');
      else
      { uint8_t Chan=MBandChan(Sec, Alt);
        printf("%c", '0'+Chan); }
    }
    printf("\n"); }

  return 0; }
