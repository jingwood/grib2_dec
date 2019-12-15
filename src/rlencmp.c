/*-------------------------------------------------------------------------- */ 
/* Last Updated Ver.1.2 2003.03.27 */ 
/* Copyright (C) 2003 Japan Meteorological Agency All rights reserved */ 
/* -------------------------------------------------------------------------- */

#include <string.h>

#include "sample_decode.h"

int ipow(const int i, const int j)  {
  int k, l;
  for (k = 0, l = 1; k<j; k++) l *= i;
  return l;
}

void endian_conv4(void *idat) {
  unsigned char *uc, c;
  uc = (unsigned char *)idat; 
  c = *(uc+0); *(uc+0) = *(uc+3); 
  c = *(uc+1); *(uc+1) = *(uc+2);
  *(uc+3) = c; *(uc+2) = c;
}

int nbit_unpack(unsigned char din[], int nin, unsigned int dout[], int nout, int nbit) {
  unsigned int wi, n_b2s, *ui;
  int i, bitp, dp;
  
  n_b2s = ipow(2,nbit) - 1;
  i = 0; dp = 0; bitp = 0;
  while (dp < nin) {
    bitp += nbit;
    memcpy(&wi, &din[dp], 4);
    
#ifdef LITTLE_ENDIAN
    dout[i] = wi & n_b2s; 
#else
    dout[i] = (wi >> (32 - bitp)) & n_b2s;
#endif

    while (bitp >= 8) { dp++; bitp -= 8; }
    if (++i>nout) return -1;
  }

  return i;
}


/*-------------------------------------------------------------------------- */
/* decode ranlength compress ( nbit data version )                           */
/* output:  udata = user data for put                                        */
/* input:   utype = user data type :sizeof(int or short or unsigned char)    */
/*          din   = compressed data                                          */ 
/*          nin   = compressed data size (byte)                */ 
/*          nout  = number of grid point                  */ 
/*          maxv  = maximum value of user data               */
/*          nbit  = number of bit used for a compressed data */
/* return: >=0    = number of decoded data */
/*         -4     = uncompressed data size exceeds nout */ 
/*         -6     = first user data is out of the data range */
/*-------------------------------------------------------------------------- */
int decode_rlen_nbit(void *udata, size_t utype, unsigned char *din, int nin,
                     int nout, int maxv, int nbit) {
  int i, j, k, l, m, n, v, p, cf = 0, *doi, ninb;
  short *dos;
  unsigned char *d, *doc;
  unsigned int *wd, *ww;

  doi = (int *)udata; dos = (short *)udata; doc = (unsigned char *)udata;
  wd = (unsigned int *)malloc(sizeof(unsigned int)*nout);
  ww = (unsigned int *)malloc(sizeof(unsigned int)*nout);
  ninb = nbit_unpack(din, nin, wd, nout, nbit);
  
  if (ninb < 0) {
    return -4;
  }

  l = ipow(2,nbit) - 1 - maxv;
  v = (int)(*wd);
  if (v < 0 || v > maxv) {
    return -6;
  }

  i = 0; k = 0; p = -1;
  while (i < ninb) {
    v = (int)(*(wd+i++));
    if (v <= maxv) {
      if (p >= 0) {
        for(j = 0; j < m; j++) {
          if (k == nout) { 
            cf = 1; break; 
          }
          *(ww+k++) = p;
        }
        if (cf == 1) break; 
      }
      p = v;
      m = 1;
      n = 0;
    } else {
      m += ipow(l, n++) * (v - maxv - 1);
    }
  }
 
  for (j=0; j<m; j++) {
    if (k==nout) break;
    *(ww+k++) = p;
  }

  switch (utype) {
    case 1: for(j=0; j<k; j++) *(doc+j) = (unsigned char)*(ww+j); break;
    case 2: for(j=0; j<k; j++) *(dos+j) = (short)*(ww+j);
    case 4: for(j=0; j<k; j++) *(doi+j) = (int)*(ww+j);
  }

  free(wd); free(ww);
  return k;
}
