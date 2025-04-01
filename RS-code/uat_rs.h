#pragma once

#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "fec/char.h"
#include "fec/rs-common.h"

class UAT_RS
{
  public:
   struct rs *rs_uplink;           // R-S code for uplink:      72+20 bytes
   struct rs *rs_adsb_short;       // R-S code for short ADS-B: 18+12 bytes
   struct rs *rs_adsb_long;        // R-S code for long ADS-B:  34+14 bytes

   static const int UPLINK_POLY=0x187;
   static const int ADSB_POLY  =0x187;
   static const int  BlockSize = 255;

   static const int  ShortPacket= 18;
   static const int  ShortParity= 12;
   static const int   LongPacket= 34;
   static const int   LongParity= 14;
   static const int UplinkPacket= 72;
   static const int UplinkParity= 20;

   UAT_RS()
   { rs_adsb_short = init_rs_char(8, ADSB_POLY,   /* fcr */ 120, /* prim */ 1,  ShortParity, BlockSize- ShortPacket- ShortParity);
     rs_adsb_long  = init_rs_char(8, ADSB_POLY,   /* fcr */ 120, /* prim */ 1,   LongParity, BlockSize-  LongPacket-  LongParity);
     rs_uplink     = init_rs_char(8, UPLINK_POLY, /* fcr */ 120, /* prim */ 1, UplinkParity, BlockSize-UplinkPacket-UplinkParity); }

  ~UAT_RS()
   { free_rs_char(rs_uplink);
     free_rs_char(rs_adsb_short);
     free_rs_char(rs_adsb_long); }

   void free_rs_char(struct rs *rs)
   { free(rs->alpha_to);
     free(rs->index_of);
     free(rs->genpoly);
     free(rs); }

   struct rs *init_rs_char(int symsize, int gfpoly, int fcr, int prim, int nroots, int pad)
   { struct rs *rs;
#include "fec/init_rs.h"
     return rs; }

   void encode_rs_char(struct rs *rs, data_t *data, data_t *parity)
   {
#include "fec/encode_rs.h"
   }

   int decode_rs_char(struct rs *rs, data_t *data, int *eras_pos=0, int no_eras=0)
   { int retval;
#include "fec/decode_rs.h"
     return retval; }

   void EncodeShort (uint8_t *Data, uint8_t *Parity) { encode_rs_char(rs_adsb_short, Data, Parity); }
   void EncodeLong  (uint8_t *Data, uint8_t *Parity) { encode_rs_char(rs_adsb_long,  Data, Parity); }
   void EncodeUplink(uint8_t *Data, uint8_t *Parity) { encode_rs_char(rs_uplink,     Data, Parity); }

    int DecodeShort (uint8_t *Data) { return decode_rs_char(rs_adsb_short, Data); }
    int DecodeLong  (uint8_t *Data) { return decode_rs_char(rs_adsb_long , Data); }
    int DecodeUplink(uint8_t *Data) { return decode_rs_char(rs_uplink    , Data); }

} ;

