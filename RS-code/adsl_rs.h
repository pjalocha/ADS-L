#pragma once

#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "fec/char.h"
#include "fec/rs-common.h"

class ADSL_RS
{
  public:
   struct rs        *rs_adsl;           // R-S code for the ADS-L
   static const int  POLY = 0x187;      // for Voyager 0x11D = x^8+x^4+x^3+X^2+1
   static const int  BlockSize = 255;
   static const int  ParitySize= 32;

   ADSL_RS()
   { rs_adsl = init_rs_char(8, POLY, /* fcr */ 120, /* prim */ 1,  ParitySize, 0); }

  ~ADSL_RS()
   { free_rs_char(rs_adsl); }

   void free_rs_char(struct rs *rs)
   { free(rs->alpha_to);
     free(rs->index_of);
     free(rs->genpoly);
     free(rs); }

   static struct rs *init_rs_char(int symsize, int gfpoly, int fcr, int prim, int nroots, int pad)
   { struct rs *rs;
#include "fec/init_rs.h"
     return rs; }

   static void encode_rs_char(struct rs *rs, data_t *data, data_t *parity)
   {
#include "fec/encode_rs.h"
   }

   static int decode_rs_char(struct rs *rs, data_t *data, int *eras_pos=0, int no_eras=0)
   { int retval;
#include "fec/decode_rs.h"
     return retval; }

   void Encode(uint8_t *Data, uint8_t DataLen)    // appends Parity after Data
   { rs_adsl->pad = BlockSize-ParitySize-DataLen;
     encode_rs_char(rs_adsl, Data, Data+DataLen); }

    int Decode(uint8_t *Data, uint8_t DataLen)    // Parity is assumed to follow Data
   { rs_adsl->pad = BlockSize-ParitySize-DataLen;
     return decode_rs_char(rs_adsl, Data); }

} ;
