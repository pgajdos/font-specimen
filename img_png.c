/*
 * font-specimen
 *
 * Display Font Specimen
 *
 * Copyright (C) 2014 Petr Gajdos (pgajdos at suse)
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include <png.h>
#include <stdio.h>

#include "img_png.h"
#include "ft.h"
#include "error.h"

char *libpng_version(char *string, int maxlen)
{
  snprintf(string, maxlen, "%d.%d.%d", 
           PNG_LIBPNG_VER_MAJOR,
           PNG_LIBPNG_VER_MINOR,
           PNG_LIBPNG_VER_RELEASE);
  return string;
}

unsigned char *swapRB(unsigned char *pix_string, int len)
{
  int p;
  char tmp;

  if (len % 3 == 0)
  {
    font_specimen_error("img_png: bitmap lenght of row is not multiple of 3");
    return NULL;
  }

  for (p = 0; p < len; p += 3)
  {
    tmp = pix_string[p];
    pix_string[p] = pix_string[p + 2];
    pix_string[p + 2] = tmp;
  }
  return pix_string;
}

int img_png_write(FILE *png, bitmap_t bitmap)
{
  int  j, png_width, png_height;
  png_structp png_ptr;
  png_infop info_ptr;
  unsigned char row[bitmap.height];

  png_width  = lay_horizontal(bitmap.ord) ? bitmap.width / 3  : bitmap.width;
  png_height = lay_vertical(bitmap.ord)   ? bitmap.height / 3 : bitmap.height;

  printf("PNG: %dx%d\n", png_width, png_height);

  png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
  if (png_ptr == NULL) 
  {
    font_specimen_error("img_png: can not create write structure");
    return -1;
  }

  info_ptr = png_create_info_struct(png_ptr);
  if (info_ptr == NULL) 
  {
    font_specimen_error("img_png: can not create info structure");
    return -1;
  }

  if (setjmp(png_jmpbuf(png_ptr))) 
  {
    font_specimen_error("img_png: can not set png error handler");
    return -1;
  }

  png_init_io(png_ptr, png);

  if (lay_color(bitmap.ord))
  {
    png_set_IHDR(png_ptr, info_ptr, png_width, png_height,
                 8, PNG_COLOR_TYPE_RGB, PNG_INTERLACE_NONE,
                 PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);
  }
  else
  {
    png_set_IHDR(png_ptr, info_ptr, png_width, png_height,
                 8, PNG_COLOR_TYPE_GRAY, PNG_INTERLACE_NONE,
                 PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);
  }

  png_write_info(png_ptr, info_ptr);
  for (j = 0; j < bitmap.height; j++)
  {
    if (lay_bgr(bitmap.ord))
    {
      memcpy(bitmap.data[j], row, bitmap.height);
      if (swapRB(row, bitmap.height) == NULL)
        return -1;
      png_write_row(png_ptr, row);
    }
    else
    {
      png_write_row(png_ptr, bitmap.data[j]);
    }
  }
  png_write_end(png_ptr, NULL);

  png_free_data(png_ptr, info_ptr, PNG_FREE_ALL, -1);
  png_destroy_write_struct(&png_ptr, (png_infopp)NULL);

  return 0;
}

