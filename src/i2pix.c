/* -------------------------------------------------------------------------- */
/* Last Updated Ver.1.2 2003.03.27 */
/* Copyright (C) 2003 Japan Meteorological Agency All rights reserved */
/* -------------------------------------------------------------------------- */

#include <stdio.h>
#include <stdlib.h>

void i2pix(
  int *fd, /* orignal data array */
  int ix,   /* size of x-axis */
  int iy,   /* size of y-axis */
  int maxval, /* maximum value of data */
  FILE *fp /* output file pointer */
) {
  int c, i, j, k, l, m;
  int r, g, b;
  char *line, *p;

  line = (char *)malloc(sizeof(char)*ix + 1);
  fprintf(fp,"static char *no_xpm[] = {\n");
  fprintf(fp,"\"%d %d %d 1\",\n", ix, iy, maxval+1);
  fprintf(fp,"\"# c #000000\",\n");
  fprintf(fp,"\"$ c #888888\",\n");
  fprintf(fp,"\"%c c #FFFFFF\",\n", '%');

  for (i = 2; i < maxval; i++) {

    if (i <= maxval / 4)            r = 0;
    else if (i <= maxval / 2)       r = (i - maxval / 4) * 4;
    else                            r = maxval;
    
    if (i <= maxval / 4)            g = i*4;
    else if (i <= maxval / 2)       g = maxval;
    else if (i <= maxval * 3 / 4)   g = maxval - (i - maxval / 2) * 4;
    else                            g = 0;

    if (i <= maxval / 4)            b = maxval;
    else if (i <= maxval / 2)       b = maxval - (i - maxval / 4) * 4;
    else if (i <= maxval * 3 / 4)   b = 0;
    else                            b = (i-maxval * 3 / 4) * 4;

    if (r < 0) r = 0; r *= 255.0 / maxval;
    if (g < 0) g = 0; g *= 255.0 / maxval;
    if (b < 0) b = 0; b *= 255.0 / maxval;

    fprintf(fp,"\"%c c #%02X%02X%02X\",\n", i+'$', r, g, b);
  }

  for (j = 0; j < iy - 10; j++) {
    p = line;
    for(i = 0; i < ix; i++) {
      k = i + j * ix;
      if (fd[k] == 0) {
        *p++ = '$';
      } else {
        *p++ = (char)((int)'$' + fd[k]);
      }
    }

    *p = '\0';
    fprintf(fp, "\"%s\",\n", line);
  }

  /* color bar */
  for (j = iy - 10; j < iy; j++) {
    p = line;
    for (i = 0; i<ix; i++) {
      *p++ = (char)((int)'$' + i * (maxval + 1) / ix);
    }
    *p = '\0';
    fprintf(fp,"\"%s\",\n", line);
  }

  fprintf(fp,"};\n");
  free(line);
}