#include <stdio.h>

#include "intmath.h"

static const uint32_t SinePoints=256;
// static const uint32_t SineScale=0x40000000;
static const int32_t SineTable[SinePoints/4+1] =
{ 0x00000000, 0x0192155F, 0x0323ECBE, 0x04B54825, 0x0645E9AF, 0x07D59396, 0x09640837, 0x0AF10A22,
  0x0C7C5C1E, 0x0E05C135, 0x0F8CFCBE, 0x1111D263, 0x1294062F, 0x14135C94, 0x158F9A76, 0x17088531,
  0x187DE2A7, 0x19EF7944, 0x1B5D100A, 0x1CC66E99, 0x1E2B5D38, 0x1F8BA4DC, 0x20E70F32, 0x223D66A8,
  0x238E7673, 0x24DA0A9A, 0x261FEFFA, 0x275FF452, 0x2899E64A, 0x29CD9578, 0x2AFAD269, 0x2C216EAA,
  0x2D413CCD, 0x2E5A1070, 0x2F6BBE45, 0x30761C18, 0x317900D6, 0x32744493, 0x3367C090, 0x34534F41,
  0x3536CC52, 0x361214B0, 0x36E5068A, 0x37AF8159, 0x387165E3, 0x392A9642, 0x39DAF5E8, 0x3A8269A3,
  0x3B20D79E, 0x3BB6276E, 0x3C42420A, 0x3CC511D9, 0x3D3E82AE, 0x3DAE81CF, 0x3E14FDF7, 0x3E71E759,
  0x3EC52FA0, 0x3F0EC9F5, 0x3F4EAAFE, 0x3F84C8E2, 0x3FB11B48, 0x3FD39B5A, 0x3FEC43C7, 0x3FFB10C1,
  0x40000000 };
/*
// static const uint32_t SineScale=0x80000000;
static const int32_t SineTable[SinePoints/4+1] =
{ 0x00000000, 0x03242ABF, 0x0647D97C, 0x096A9049, 0x0C8BD35E, 0x0FAB272B, 0x12C8106F, 0x15E21445,
  0x18F8B83C, 0x1C0B826A, 0x1F19F97B, 0x2223A4C5, 0x25280C5E, 0x2826B928, 0x2B1F34EB, 0x2E110A62,
  0x30FBC54D, 0x33DEF287, 0x36BA2014, 0x398CDD32, 0x3C56BA70, 0x3F1749B8, 0x41CE1E65, 0x447ACD50,
  0x471CECE7, 0x49B41533, 0x4C3FDFF4, 0x4EBFE8A5, 0x5133CC94, 0x539B2AF0, 0x55F5A4D2, 0x5842DD54,
  0x5A82799A, 0x5CB420E0, 0x5ED77C8A, 0x60EC3830, 0x62F201AC, 0x64E88926, 0x66CF8120, 0x68A69E81,
  0x6A6D98A4, 0x6C242960, 0x6DCA0D14, 0x6F5F02B2, 0x70E2CBC6, 0x72552C85, 0x73B5EBD1, 0x7504D345,
  0x7641AF3D, 0x776C4EDB, 0x78848414, 0x798A23B1, 0x7A7D055B, 0x7B5D039E, 0x7C29FBEE, 0x7CE3CEB2,
  0x7D8A5F40, 0x7E1D93EA, 0x7E9D55FC, 0x7F0991C4, 0x7F62368F, 0x7FA736B4, 0x7FD8878E, 0x7FF62182,
  0x7FFFFFFF } ;
*/

// get Sine from the SineTable
// Angle is 0..255 which corresponds to <0..2*PI)
int32_t IntSine(uint8_t Angle)
{ uint8_t Idx=Angle;
  if(Angle&0x80) { Angle^=0x40; Idx=(-Idx); }
  if(Angle&0x40) Idx=0x80-Idx;
  int32_t Val=SineTable[Idx];
  if(Angle&0x80) Val=(-Val);
  return Val; }

// precise Sine with for 16-bit angles 2nd derivative interpolation
// max. result error is about 2.3e-7
int32_t IntSine(uint16_t Angle)
{ uint8_t Int    = Angle>>8;                                  // upper 8-bits of the angle
  int32_t Frac   = Angle&0x00FF;
  int32_t Value  = IntSine(Int); Int+=1;                      // approx. sin()
  int32_t Delta  = (IntSine(Int)-Value+0x80)>>8;              // sin() of the next step in the table, Delta
          Value += Frac*Delta;                                // linear approx. between the table points
  // printf("\n IntSine(%04X) [Int=%02X:%02X Frac=%02X Value=%+11.8f]\n", Angle, Int-1, Int, Frac, (double)Value/(uint32_t)0x80000000);
  int32_t Frac2  = (Frac*(Frac-0x100));                       // quadratic between the table points
  const int32_t Coeff = (int32_t)floor(2*M_PI*M_PI*0x80+0.5); // 0x80
  int32_t Deriv2 = (Coeff*((Value+0x800)>>12));               // >>12
  int32_t Corr   = (((Deriv2+0x2000)>>14)*Frac2+0x1000)>>13;  // >>16 >>11
          Value -= Corr;
  // printf("\n IntSine(%04X) [%04X %+11.8f %+11.8f]\n", Angle, -Frac2, (double)Deriv2/(uint32_t)0x80000000, (double)Corr/(uint32_t)0x80000000);
  return Value; }                                             // accuracy: RMS 0.1ppm, max. 0.3ppm

// precise Sine for 32-bit angles with 2nd derivative interpolation
// max. result error is about 2.3e-7
int32_t IntSine(uint32_t Angle)
{ uint8_t  Int   = Angle>>24;
  int32_t Frac   = Angle&0x00FFFFFF;
  int32_t Value  = IntSine(Int); Int+=1;
  int32_t Delta  = (IntSine(Int)-Value);
          Value += ((int64_t)Frac*(int64_t)Delta+0x00800000)>>24;
  // printf(" [%02X %06X %+11.8f] ", Int-1, Frac, (double)Value/(uint32_t)0x80000000);
  int64_t Frac2  = ((int64_t)Frac*(Frac-0x1000000)+0x80000000)>>32;
  const int64_t Coeff = (int64_t)floor(2*M_PI*M_PI*0x4000000+0.5);
  int64_t Deriv2 = (Coeff*Value +0x02000000u)>>26;   // >>26
  int64_t Corr   = (Deriv2*Frac2+0x80000000u)>>32;   // >>32
         Value  -= Corr;
  // printf(" [%04X %+11.8f %+11.8f] ",  -Frac2, (double)Deriv2/(uint32_t)0x80000000, (double)Corr/(uint32_t)0x80000000);
  return Value; }

// Less precise sine for 16-bit angles
// source: http://www.coranac.com/2009/07/sines/
/// A sine approximation via a fourth-order cosine approx.
/// @param x   angle (with 2^16 units/circle)
/// @return     Sine value (Q12)
int16_t Isin(int16_t Angle)     // input: full angle = 16-bit range
{
    int32_t x=Angle;
    int32_t c, y;
    static const int qN= 14, qA= 12, B=19900, C=3516;

    c= x<<(30-qN);              // Semi-circle info into carry.
    x -= 1<<qN;                 // sine -> cosine calc

    x= x<<(31-qN);              // Mask with PI
    x= x>>(31-qN);              // Note: SIGNED shift! (to qN)
    x= (x*x)>>(2*qN-14);        // x=x^2 To Q14

    y= B - ((x*C)>>14);         // B - x^2*C
    y= (1<<qA)-((x*y)>>16);     // A - x^2*(B-x^2*C)

    return c>=0 ? y : -y; }     // result: -4096..+4096 (max. error = +/-12)
/*
// source: http://www.electroyou.it/forum/viewtopic.php?t=56108
    uint32_t isin_S5(uint32_t x)
    { uint32_t tmp, x2;
      x2 = x * x;
      tmp = (x2 * 145) >> (2*N);
      tmp = 1314 - tmp;
      tmp *= x;
      tmp >>= N;
      tmp *= x;
      tmp >>= N;
      tmp = 3217 - tmp;
      tmp *= x;
      tmp >>= 9;
      if (tmp > 0x3FFF) return 0x3FFF;
      return tmp; }

    uint32_t Isin(uint32_t angle)
    {
           if (angle < 0x1000)
      { return 0x4000 + isin_S5(angle); }
      else if (angle < 0x2000)
      { return  0x4000 + isin_S5(0x2000 - angle); }
      else if (angle < 0x3000)
      { return 0x4000 - isin_S5(angle - 0x2000); }
      else
      { return 0x4000 - isin_S5(0x4000 - angle); }
    }
*/

/*
int16_t IntAtan2(int16_t Y, int16_t X)
{ uint16_t Angle=0;                                             // printf(" [%+5d,%+5d] %04X\n", X, Y, Angle);
  if(Y<0)     { Angle+=0x8000; X=(-X); Y=(-Y); }                // printf(" [%+5d,%+5d] %04X\n", X, Y, Angle);
  if(X<0)     { Angle+=0x4000; int16_t tmp=Y; Y=(-X); X=tmp;  } // printf(" [%+5d,%+5d] %04X\n", X, Y, Angle);
  if(X<Y)     { Angle+=0x4000; int16_t tmp=Y; Y=(-X); X=tmp;  } // printf(" [%+5d,%+5d] %04X\n", X, Y, Angle);
  if(X==0) return 0;
  int16_t D = (((int32_t)Y<<14) + (X>>1))/ X;
  int16_t DD = ((int32_t)D*(int32_t)D)>>14;
  int16_t DDD = ((int32_t)DD*(int32_t)D)>>14;
  // printf(" %08X %08X %08X\n", D, DD, DDD);
  Angle += ((5*D)>>3) - (DDD>>3); return Angle; } // good to about 1/2 degree
*/

int16_t IntAtan2(int16_t Y, int16_t X)
{ uint16_t Angle=0;                                             // printf(" [%+5d,%+5d] %04X\n", X, Y, Angle);
  const int32_t CosPi8 = 30274; // cos(PI/8)*32768
  const int32_t SinPi8 = 12540; // sin(PI/8)*32768
  if(Y<0)     { Angle+=0x8000; X=(-X); Y=(-Y); }                // printf(" [%+5d,%+5d] %04X\n", X, Y, Angle);
  if(X<0)     { Angle+=0x4000; int16_t tmp=Y; Y=(-X); X=tmp;  } // printf(" [%+5d,%+5d] %04X\n", X, Y, Angle);
  if(X<Y)     { Angle+=0x4000; int16_t tmp=Y; Y=(-X); X=tmp;  } // printf(" [%+5d,%+5d] %04X\n", X, Y, Angle);
  if(X==0) return 0;
  int16_t Yc = (Y<<1)+(Y>>1);
  if(Y<0)
  { if(X<(-Yc))
    { int32_t NewX =    CosPi8*X - SinPi8*Y;
      int32_t NewY =    SinPi8*X + CosPi8*Y;
      X=NewX>>15; if(NewX&0x4000) X+=1;
      Y=NewY>>15; if(NewY&0x4000) Y+=1;
      Angle-=0x1000; }
  } else // Y>=0
  { if(X<Yc)
    { int32_t NewX =    CosPi8*X + SinPi8*Y;
      int32_t NewY =  - SinPi8*X + CosPi8*Y;
      X=NewX>>15; if(NewX&0x4000) X+=1;
      Y=NewY>>15; if(NewY&0x4000) Y+=1;
      Angle+=0x1000; }
  }                                                             // printf(" [%+5d,%+5d] %04X\n", X, Y, Angle);
  int16_t D = (((int32_t)Y<<14) + (X>>1))/ X;
  // int16_t D = ((int32_t)Y<<14) / X;
  int16_t DD = ((int32_t)D*(int32_t)D)>>14;
  int16_t DDD = ((int32_t)DD*(int32_t)D)>>14;
  // printf(" %08X %08X %08X\n", D, DD, DDD);
  Angle += ((5*D)>>3) - (DDD>>3);
  return Angle; } // good to about 1/6 degree


int16_t IntAtan2(int32_t Y, int32_t X)
{ for( ; ; )
  { if( X<32768 && X>=(-32768) && Y<32768 && Y>=(-32768) ) break;
    X>>=1; Y>>=1; }
  return IntAtan2((int16_t)Y, (int16_t)X); }

/*
// integer square root
uint32_t IntSqrt(uint32_t Inp)
{ uint32_t Out  = 0;
  uint32_t Mask = 0x40000000;

  while(Mask>Inp) Mask>>=2;
  while(Mask)
  { if(Inp >= (Out+Mask))
    { Inp -= Out+Mask; Out += Mask<<1; }
    Out>>=1; Mask>>=2; }
  if(Inp>Out) Out++;

  return Out; }

uint64_t IntSqrt(uint64_t Inp)
{ uint64_t Out  = 0;
  uint64_t Mask = 0x4000000000000000;

  while(Mask>Inp) Mask>>=2;
  while(Mask)
  { if(Inp >= (Out+Mask))
    { Inp -= Out+Mask; Out += Mask<<1; }
    Out>>=1; Mask>>=2; }
  if(Inp>Out) Out++;

  return Out; }
*/
