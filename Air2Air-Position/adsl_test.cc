#include <stdio.h>

#include "adsl.h"

static ADSL_Packet Packet;
static char Line[128];

int main(int argc, char *argv[])
{ Packet.Init();
  Packet.setAddress(0x123456);
  Packet.setAddrTable(8);
  Packet.TimeStamp=4*4;                           // [0.25 sec]
  Packet.setLat(Packet.UBXtoFNT(481234500));      // 48.12345 deg
  Packet.setLon(Packet.UBXtoFNT( 81234500));      //  8.12345 deg
  Packet.setAlt(1234);                            // 1234 m
  Packet.setSpeed(45*4);                          // 45.0 m/s
  Packet.setClimb(-5*8-4);                        // -5.5 m/s
  Packet.setTrack(0x100);                         // 180.0 deg

  Packet.Dump(Line);
  printf("Before Scramble() and CRC %s\n", Line);

  Packet.Scramble();
  Packet.setCRC();

  Packet.Dump(Line);
  printf("After  Scramble() and CRC %s\n", Line);

  printf("checkCRC() => %06X\n", Packet.checkCRC());
  Packet.Descramble();

  Packet.Dump(Line);
  printf("After Descramble()        %s\n", Line);

  Packet.Print();

  return 0; }

