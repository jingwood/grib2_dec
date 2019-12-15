#include <string.h>

#include "sample_decode.h"
#include "i2pix.h"

void print_info(ST_SECT ss[], int sn) {
  int i, j, k;
  unsigned long long *ull;
  int *ii;
  unsigned short *us, *n;

  printf("========== SECTION %1.1d ===========\n", (sn >= 2) ? sn + 1 : sn);
  for (i = 0, j=1; i < ss[sn].num; j += *(ss[sn].len+i), i++) {
    if (*(ss[sn].len+i) == 1) {
      if (**(ss[sn].v+i) == 0xff) {
        printf(" %3d : 0x%2.2x\n", j, **(ss[sn].v+i));
      } else {
        printf(" %3d : %d\n", j, **(ss[sn].v+i));
      }
    } else if (*(ss[sn].len+i) == 8) { 
      if (sn == 0) {
        ull = (unsigned long long *)*(ss[sn].v+i);
        printf("%4d --%4d: %d\n", j,j+*(ss[sn].len+i)-1, (unsigned int)ull); 
      } else {
        printf("%4d --%4d: ", j,j+*(ss[sn].len+i)-1);
        for (k=0; k<8; k++) {
          printf("%2.2x", *(*(ss[sn].v+i)+k)); 
        }
        printf("\n");
      }
    } else if (*(ss[sn].len + i) == 4) {
      ii = (int *)*(ss[sn].v + i);
      if (*ii >= 0 || (i == 12 && sn == 3)) /* only fcst_time is signed int */ {
        printf("%4d --%4d: %d\n", j,j+*(ss[sn].len+i)-1, *ii); 
      } else {
        printf("%4d --%4d: 0x%8.8x\n", j,j+*(ss[sn].len+i)-1, *ii);
      }
    } else if (*(ss[sn].len+i) == 2) {
      us = (unsigned short *)*(ss[sn].v+i);
      if (*us == 0xffff) {
        printf("%4d --%4d: 0x%4.4x\n", j, j+*(ss[sn].len+i)-1, *us); 
      } else {
        printf("%4d --%4d: %d\n", j, j+*(ss[sn].len+i)-1, *us);
      }
    } else if (sn == 3 || sn == 4) {
      n = (unsigned short *)*(ss[sn].v + i - 2);
      us = (unsigned short *)*(ss[sn].v + i);
      for (k = 0; k < *n; k++) {
        if (*(us+k)==0xffff) {
          printf("%4d --%4d: 0x%4.4x\n", j+2*k,j+2*k+1, *(us+k));
        } else {
          printf("%4d --%4d: %d\n", j+2*k,j+2*k+1, *(us+k));
        }
      }
    }
  }
  fflush(stdout);
}

int main(int argc, char *argv[]) {
  ST_SECT ss[8];
  FILE *fp, *fpo;
  char fname[160], gname[160], suffix[160], fcs[160], ffm[160];
  int sn, *lv, gn, sff = 0, *xs, *ys, maxv, fcnt = 0, af, ll;
  unsigned short *us_maxv;

  if (argc <= 1) {
    fprintf(stderr, 
    "\nThis program decodes the grib2 file named ***_grib2.bin, and prints the value\n"
    "of each section in GRIB2. Also, this program puts out a raw (4 byte integer)\n"
    "data file ***_int.bin as a rectangle grid dimension. In case of specifying -xpm\n"
    "options, an output fileis to a picture image file ***.xpm formatted as X-pixmap.\n");
    fprintf(stderr, "\nusage: grib2_dec ***_grib2.bin [-xpm]\n\n");

    exit(1);
  } else if (argc == 3 && strcmp(argv[2],"-xpm") == 0) {
    strcpy(suffix, ".xpm"); sff = 1;
  } else {
    strcpy(suffix, ".bin");
  }

  strcpy(fname, argv[1]);
  if ((fp = fopen(fname,"r")) == NULL) {
    fprintf(stderr, "grib2 file <%s> open error!!\n", fname);
    exit(1);
  }
  
  af = (strstr(fname, "_ANAL") == NULL) ? 1 : 0;

  init_sect(ss, af);

  while ((sn = read_sect(ss, fp)) != 8) {
    if (sn == 6) {
      print_info(ss, sn);
      gn = dec_data(ss, &lv); 
      if (gn > 0) {
        ll = strlen(fname) - strlen(strstr(fname, "_grib2.bin"));
        strncpy(gname, fname, ll);
        gname[ll] = '\0';
        sprintf(fcs, "_%1d", fcnt);
        strcpy(ffm, (sff == 1) ? "" : "_int"); 
        strcat(gname, strcat(fcs, strcat(ffm, suffix)));
        
        if ((fpo = fopen(gname, "w")) == NULL) {
          fprintf(stderr, "output file <%s> open error!!\n", gname);
          exit(1);
        }

        if (sff == 0) {
          fwrite(lv, sizeof(int), gn, fpo);
        } else {
          xs = (int *)*(ss[2].v + 14);
          ys = (int *)*(ss[2].v + 15);
          us_maxv = (unsigned short *) * (ss[4].v+6); maxv = (int)*us_maxv;
          i2pix(lv, *xs, *ys, maxv, fpo);
        }
        fclose(fpo);
        fcnt++;
      }
      free(lv);
    } else {
      print_info(ss, sn);
    }
  }

  fclose(fp);
}