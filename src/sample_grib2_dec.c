
/* -------------------------------------------------------------------------- */
/* Last Updated Ver.1.2 2003.03.27 */
/* Copyright (C) 2003 Japan Meteorological Agency All rights reserved */
/* -------------------------------------------------------------------------- */

#include <string.h>

#include "sample_decode.h"
#include "i2pix.h"
#include "prr_template.h"
#include "pmf_template.h"

#ifdef LITTLE_ENDIAN
  #define Fread fread_little_endian
#else
  #define Fread fread
#endif

void fread_little_endian(void *d, int len, int num, FILE *fp) {
  unsigned char uc[8], *ud;
  int i, j, k;

  ud = d;
  if (len == 1) {
    fread(d, len, num, fp); 
  } else {
    for (i=0, k=0; i<num; i++, k+=len) {
      fread(uc, len, 1, fp);
      for (j=0; j<len; j++) {
        *(ud+k+j) = uc[len-1-j];
      }
    }
  }
}

void init_sect(ST_SECT ss[], int af) {
  int i, j, k;
  char c[2];
  
  if (af == 0) {
    k = 0; while (strlen(sc_prr[k]) != 0) k++;

    for (i = 0; i < k; i++) {
      ss[i].num = strlen(sc_prr[i]);
      ss[i].len = (int *)malloc(sizeof(int *)*ss[i].num);
  
      for (j = 0; j < ss[i].num; j++) {
        strncpy(c, &sc_prr[i][j], 1); *(ss[i].len+j) = atoi(c);
      }
      ss[i].v = (unsigned char **)malloc(sizeof(unsigned char *)*ss[i].num);
    }
  } else {
    k=0; while (strlen(sc_pmf[k])!=0) k++; 
    
    for (i=0; i<k; i++) {
      ss[i].num = strlen(sc_pmf[i]);
      ss[i].len = (int *)malloc(sizeof(int *)*ss[i].num);
      
      for (j=0; j<ss[i].num; j++) {
        strncpy(c, &sc_pmf[i][j], 1); *(ss[i].len+j) = atoi(c); 
      }
      
      ss[i].v = (unsigned char **)malloc(sizeof(unsigned char *)*ss[i].num);
      for(j=0; j<ss[i].num; j++) *(ss[i].v+j) = NULL; 
    }
  }
}

int read_sect(ST_SECT ss[], FILE *fp) {
  int i, j, nn, mm, si;
  unsigned int slen;
  unsigned short *us, ud;
  unsigned char sn;

  Fread(&slen, 4, 1, fp);
  if (memcmp(&slen, "7777", 4)==0) {
    si = 8;
    return si;

#ifdef LITTLE_ENDIAN
  } else if (memcmp(&slen, "BIRG", 4)==0) {
#else
  } else if (memcmp(&slen, "GRIB", 4)==0) {
#endif

    si = 0; slen = 16;
    Fread(&ud, 2, 1, fp);
    *(ss[si].v+0)=(unsigned char *)realloc(*(ss[si].v+0),sizeof(unsigned int));
    *(ss[si].v+1)=(unsigned char *)realloc(*(ss[si].v+1),sizeof(unsigned short));
    memcpy(*(ss[si].v+0), &slen, 4);
  } else {
    Fread(&sn, 1, 1, fp);
    si = (int)sn;
    if (si >= 3) si--;
    *(ss[si].v+0)=(unsigned char *)realloc(*(ss[si].v+0),sizeof(unsigned int));
    *(ss[si].v+1)=(unsigned char *)realloc(*(ss[si].v+1),sizeof(unsigned char));
    memcpy(*(ss[si].v+0), &slen, 4);
    memcpy(*(ss[si].v+1), &sn, 1);
  }

  for (i = 2; i < ss[si].num; i++) {
    if (*(ss[si].len + i)==0) {
      us = (unsigned short *)(*(ss[si].v + i-2));
      nn = (*(ss[si].len + i-2)==4) ? sizeof(unsigned char) : sizeof(unsigned short);
      mm = (*(ss[si].len + i-2)==4) ? slen-5 : *us;
    } else {
      nn = *(ss[si].len + i);
      mm = 1;
    }
    *(ss[si].v+i) = (unsigned char *)realloc(*(ss[si].v+i), (size_t)nn*mm);
    Fread(*(ss[si].v+i), nn, mm, fp);
  }

  return si;
}

int dec_data(ST_SECT ss[], int **lv) {
  int nin, nout, maxv, nbit, rt;
  unsigned int *ui;
  unsigned short *us;
  ui = (unsigned int *)*(ss[4].v + 2);    nout = *ui;
  nbit = **(ss[4].v + 4);
  us = (unsigned short *)*(ss[4].v + 5);  maxv = *us;
  ui = (unsigned int *)*(ss[6].v + 0);    nin = *ui - 5;

  *lv = (int*)malloc(sizeof(int)*nout);
  rt = decode_rlen_nbit(*lv, sizeof(int), *(ss[6].v + 2), nin, nout, maxv, nbit);

  return rt;
}


