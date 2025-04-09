#include <stdint.h>
#include <string.h>
#include <math.h>

#include <Arduino.h>

#include <RadioLib.h>

#include "manchester.h"

#include "adsl.h"

// ===============================================================================================================

uint64_t getUniqueMAC(void)                                  // 48-bit unique ID of the ESP32 chip
{ uint8_t MAC[6]; esp_efuse_mac_get_default(MAC);
  uint64_t ID=MAC[0];
  for(int Idx=1; Idx<6; Idx++)
  { ID<<=8; ID|=MAC[Idx]; }
  return ID; }

uint64_t getUniqueID     (void) { return getUniqueMAC(); }            // get unique serial ID of the CPU/chip
uint32_t getUniqueAddress(void) { return getUniqueMAC()&0x00FFFFFF; } // get unique OGN address

// ===============================================================================================================

// those are for TTGO modules
#define Radio_PinRST  23 // Reset
#define Radio_PinCS   18 // CS
#define Radio_PinSCK   5 // SCK
#define Radio_PinMOSI 27 // MOSI
#define Radio_PinMISO 19 // MISO
#define Radio_PinIRQ  26 // IRQ
#define Radio_PinBusy 32 // Busy: only for SX1262

#ifdef WITH_SX1276
SX1276 Radio = new Module(Radio_PinCS, Radio_PinIRQ, Radio_PinRST, -1);
#endif
#ifdef WITH_SX1262
SX1262 Radio = new Module(Radio_PinCS, Radio_PinIRQ, Radio_PinRST, Radio_PinBusy);    // create sx1262 RF module
#endif

bool Radio_IRQ(void) { return digitalRead(Radio_PinIRQ); }

int Radio_Init(void)
{ SPI.begin(Radio_PinSCK, Radio_PinMISO, Radio_PinMOSI);  // CLK, MISO, MOSI, CS given by the Radio contructor
#ifdef WITH_SX1276
  int State = Radio.beginFSK(868.2,          100.0,           50.0,        234.3,           14,              8);
#endif
#ifdef WITH_SX1262
  int State = Radio.beginFSK(868.2,          100.0,           50.0,        234.3,            0,              8,           1.6,       0);
  State = Radio.setFrequency(868.8, 1); // calibrate
  State = Radio.setTCXO(1.6);
  State = Radio.setDio2AsRfSwitch();
  // Radio.setDio1Action(IRQcall);
#endif
  return State; }

int Radio_ConfigManchFSK(uint8_t PktLen, const uint8_t *SYNC, uint8_t SYNClen)         // Radio setup for M-band OGN/ADS-L
{ int ErrState=0; int State=0;
#ifdef WITH_SX1276
  State=Radio.setActiveModem(RADIOLIB_SX127X_FSK_OOK);
#endif
#ifdef WITH_SX1262
  State=Radio.config(RADIOLIB_SX126X_PACKET_TYPE_GFSK);
#endif
  if(State) ErrState=State;
  State=Radio.setBitRate(100.0);                                    // [kpbs] 100kbps bit rate but we transmit Manchester encoded thus effectively 50 kbps
  if(State) ErrState=State;
  State=Radio.setFrequencyDeviation(50.0);                          // [kHz]  +/-50kHz deviation
  if(State) ErrState=State;
  State=Radio.setRxBandwidth(234.3);                                // [kHz]  250kHz bandwidth (but is it a single or double sideband ?)
  if(State) ErrState=State;
  State=Radio.setEncoding(RADIOLIB_ENCODING_NRZ);
  if(State) ErrState=State;
  State=Radio.setPreambleLength(8);                                 // [bits] minimal preamble
  if(State) ErrState=State;
  State=Radio.setDataShaping(RADIOLIB_SHAPING_0_5);                 // [BT]   FSK modulation shaping
  if(State) ErrState=State;
  State=Radio.setCRC(0, 0);                                         // disable CRC: we do it ourselves
  if(State) ErrState=State;
  State=Radio.fixedPacketLengthMode(PktLen*2);                      // [bytes] Fixed packet size mode
  if(State) ErrState=State;
#ifdef WITH_SX1276
  State=Radio.disableAddressFiltering();                            // don't want any of such features
  State=Radio.invertPreamble(true); // true=0xAA, false=0x55
#endif
  State=Radio.setSyncWord((uint8_t *)SYNC, SYNClen);                // SYNC sequence: 8 bytes which is equivalent to 4 bytes before Manchester encoding
  if(State) ErrState=State;
#ifdef WITH_SX1262
  State=Radio.setRxBoostedGainMode(true);                           // 2mA more current but boosts sensitivity
  if(State) ErrState=State;
#endif
  return ErrState; }                                                // this call takes 18-19 ms

static int ManchEncode(uint8_t *Out, const uint8_t *Inp, uint8_t InpLen) // Encode packet bytes as Manchester
{ int Len=0;
  for(int Idx=0; Idx<InpLen; Idx++)                // loop over bytes and encode usinglookup table
  { uint8_t Byte=Inp[Idx];                         // data byte to be encoded
    Out[Len++]=ManchesterEncode[Byte>>4];          // use lookup table to encode upper nibble
    Out[Len++]=ManchesterEncode[Byte&0x0F]; }      // encode lower nibble
  return Len; }                                    // returns number of bytes in the encoded packet

static uint8_t Radio_TxPacket[64];                 // Manchester-encoded packet just before transmission
// static uint8_t Radio_RxPacket[64];                 // Manchester-encoded packet just after reception

#ifdef WITH_SX1276
static int Radio_TxFSK(const uint8_t *Packet, uint8_t Len)
{ int State=Radio.startTransmit((const uint8_t *)Packet, Len);
  uint32_t usStart = micros();                                         // [usec] when transmission started
  uint32_t usTxTime=Radio.getTimeOnAir(Len);                           // [usec] predicted transmission time
   int32_t usLeft = usTxTime;                                          // [usec]
  for( ; ; )
  { uint32_t usTime = micros()-usStart;                                // [usec] time since transmission started
    usLeft = usTxTime-usTime;                                          // [usec] time left till the end of packet
    if(Radio_IRQ()) break;                                             // raised IRQ => end-of-data
    if(usLeft>1500) { delay(1); continue; }
    if(usLeft<(-40)) break;
    taskYIELD(); }
  // State=Radio.finishTransmit();                                        // adds a long delay and leaves a significant tail
  Radio.clearIrqFlags(RADIOLIB_SX127X_FLAGS_ALL);
  // Radio.clearIRQFlags();
  Radio.standby();
  return State; }

int Radio_TxManchFSK(const uint8_t *Packet, uint8_t Len)               // transmit a packet using Manchester encoding
{ int TxLen=ManchEncode(Radio_TxPacket, Packet, Len);                  // Manchester encode
  return Radio_TxFSK(Radio_TxPacket, TxLen); }
#endif

#ifdef WITH_SX1262
int Radio_TxFSK(const uint8_t *Packet, uint8_t Len)                      // transmit a packet on the O-Band
{ // Serial.printf("TxFSK[%d]", Len);
  // for(uint8_t Idx=0; Idx<Len; Idx++)
  //   Serial.printf(" %02X", Packet[Idx]);
  // Serial.printf("\n");
  int State=Radio.transmit(Packet, Len);                                 // transmit
  return State; }                                                        // this call takes 15-16 ms although the actuall packet tran>

int Radio_TxManchFSK(const uint8_t *Packet, uint8_t Len)                 // transmit a packet using Manchester encoding
{ // Serial.printf("TxManch[%d]", Len);
  // for(uint8_t Idx=0; Idx<Len; Idx++)
  //   Serial.printf(" %02X", Packet[Idx]);
  // Serial.printf("\n");
  int TxLen=ManchEncode(Radio_TxPacket, Packet, Len);                    // Manchester encode
  // uint32_t Time=millis();
  int State=Radio.transmit(Radio_TxPacket, TxLen);                       // transmit
  // Time = millis()-Time;
  // Serial.printf("Radio_TxManchFSK(, %d=>%d) (%d) %dms\n", Len, TxLen, State, Time);  // for debug
  return State; }                                                        // this call takes 15-16 ms although the actuall packet tran>
#endif

// ===============================================================================================================

// ADS-L SYNC:       0xF5724B18 encoded in Manchester (fixed packet length 0x18 is included)
static const uint8_t ADSL_SYNC[10] = { 0x55, 0x99, 0x95, 0xA6, 0x9A, 0x65, 0xA9, 0x6A, 0x00, 0x00 };


// ===============================================================================================================

const float Lat = +52.00000;           // [deg]
const float Lon =  -1.00000;           // [deg]
const int   Alt =       100;           // [m] AMSL
const int   GeoidSepar = 48;           // [m] Geoid separation

uint32_t Address  = 0;
uint32_t AddrType = 7;                 // address table: OGN
uint8_t AcftType  = 2;                 // OGN aircraft-type: motor aircraft

const float TxPower = 10.0;            // [dBm]

const float FreqM[2] = { 868.200, 868.400 } ;    // two channels on the M-Band spaced by 200kHz
const float FreqO[5] = { 869.425, 869.475, 869.525, 869.575, 869.625 } ;  // 5 channel in the O-band spaced by 50kHz

void EncodePos(ADSL_Packet &ADSL_Pos, uint8_t TimeStamp)
{ ADSL_Pos.Init();
  ADSL_Pos.setAddress(Address);
  ADSL_Pos.setAddrTable(AddrType);
  ADSL_Pos.setRelay(0);
  ADSL_Pos.setAcftTypeOGN(AcftType);
  ADSL_Pos.setLat(Coord_UBXtoFNT(round(1e7*Lat)));
  ADSL_Pos.setLon(Coord_UBXtoFNT(round(1e7*Lon)));
  ADSL_Pos.setAlt(Alt+GeoidSepar);
  ADSL_Pos.TimeStamp=TimeStamp;
  ADSL_Pos.FlightState=1;
  ADSL_Pos.Scramble();
  ADSL_Pos.setCRC(); }

// ===============================================================================================================

void setup()
{
#ifdef Reset_Pin
  pinMode(Reset_Pin, OUTPUT);
  digitalWrite(Reset_Pin, HIGH);
  delay(10);
  digitalWrite(Reset_Pin, LOW);
#endif
  Address = getUniqueAddress();

  Serial.begin(115200);
  Serial.println("ADS-L demo transmitter for ISM M-band");

  int State = Radio_Init();
  Serial.printf("Radio_Init() => %d\n", State);

}

static uint8_t TimeStamp = 0;
const uint8_t PktLen = ADSL_Packet::TxBytes-3;

void loop()
{
  ADSL_Packet ADSL_Pos;
  EncodePos(ADSL_Pos, TimeStamp);
  TimeStamp+=1; if(TimeStamp>60) TimeStamp-=60;

  for(int Chan=0; Chan<2; Chan++)
  { delay(250);
    Radio.standby();
    Radio_ConfigManchFSK(PktLen, ADSL_SYNC, 8);
    Radio.setFrequency(FreqM[Chan]);
    Radio.setOutputPower(TxPower);
    const uint8_t *Packet = &(ADSL_Pos.Version);
    Radio_TxManchFSK(Packet, PktLen);
    Serial.printf("%7.3fMHz <= ", FreqM[Chan]);
    for(uint8_t Idx=0; Idx<PktLen; Idx++)
    { Serial.printf("%02X", Packet[Idx]); }
    printf("\n");
  }

}
