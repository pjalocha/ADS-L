#include <stdint.h>
#include <string.h>
#include <math.h>

#include <Arduino.h>

// #include <RadioLib.h>

#include "adsl.h"

void setup()
{
  Serial.begin(115200);
  Serial.println("ADS-L related benchmarks on ESP32 CPU");
}

ADSL_Packet Packet;
char Line[128];

void loop()
{ const int Runs=100000;

  uint32_t Time=millis();
  for(int Run=0; Run<Runs; Run++)
    Packet.Scramble();
  Time = millis()-Time;
  Packet.Dump(Line);
  Serial.printf("Packet: %s\n", Line);
  Serial.printf("ADS-L position packet scrambling: %5.3f usec/packet\n", 1e3*Time/Runs);
}

