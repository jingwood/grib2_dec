/* **************************************** */
/* sample grib2 decode program include file */
/* **************************************** */

/* #define LITTLE_ENDIAN */
#include <stdio.h>
#include <stdlib.h>

int decode_rlen_nbit(void *udata, size_t utype, unsigned char *din, int nin,
                     int nout, int maxv, int nbit);

typedef struct sect {
  int num;
  int *len;
  unsigned char **v;
} ST_SECT;

void init_sect(ST_SECT ss[], int af);
int read_sect(ST_SECT ss[], FILE *fp);
int dec_data(ST_SECT ss[], int **lv);
