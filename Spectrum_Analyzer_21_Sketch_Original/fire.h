/********************************************************************************************************************************
*
*  Project:  21 Band Spectrum Analyzer 
*  Target Platform: ESP32 38-PIN DEV Board
*  Version: 8.0
*  
********************************************************************************************************************************/
 
 /*  This part of the code ( the fire part) has been adapted and heavly alterted from
 * Patrick Rigney (https://www.toggledbits.com/)
 * Github: https://github.com/toggledbits/MatrixFireFast */

#pragma once
#include "Settings.h"
#define MAT_W   kMatrixWidth           /* Size (columns) of entire matrix */
#define MAT_H   kMatrixHeight          /* and rows */
const uint16_t rows = MAT_H;
const uint16_t cols = MAT_W;
const uint16_t xorg = 0;
const uint16_t yorg = 0;
uint8_t pix[rows][cols];
const uint8_t NCOLORS = (sizeof(colors0)/sizeof(colors0[0]));
uint8_t nflare = 0;
uint32_t flare[maxflare];
const uint8_t phy_h = MAT_W;
const uint8_t phy_w = MAT_H;
unsigned long t = 0; /* keep time */

uint16_t pos( uint16_t col, uint16_t row ) {
    uint16_t phy_x = xorg + (uint16_t) row;
    uint16_t phy_y = yorg + (uint16_t) col;
  return phy_x + phy_y * phy_w;
}
uint32_t isqrt(uint32_t n) {
  if ( n < 2 ) return n;
  uint32_t smallCandidate = isqrt(n >> 2) << 1;
  uint32_t largeCandidate = smallCandidate + 1;
  return (largeCandidate*largeCandidate > n) ? smallCandidate : largeCandidate;
}
void glow( int x, int y, int z ) {
  int b = z * 10 / flaredecay + 1;
  for ( int i=(y-b); i<(y+b); ++i ) {
    for ( int j=(x-b); j<(x+b); ++j ) {
      if ( i >=0 && j >= 0 && i < rows && j < cols ) {
        int d = ( flaredecay * isqrt((x-j)*(x-j) + (y-i)*(y-i)) + 5 ) / 10;
        uint8_t n = 0;
        if ( z > d ) n = z - d;
        if ( n > pix[i][j] ) { // can only get brighter
          pix[i][j] = n;
        }
      }
    }
  }
}

void newflare() {
  if ( nflare < maxflare && random(1,101) <= flarechance ) {
    int x = random(0, cols);
    int y = random(0, flarerows);
    int z = NCOLORS - 1;
    flare[nflare++] = (z<<16) | (y<<8) | (x&0xff);
    glow( x, y, z );
  }
}
