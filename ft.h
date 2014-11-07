/*
 * font-specimen
 *
 *  Display Font Specimen
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

#ifndef FT_H
# define FT_H

#include <stdint.h>

#include <fontconfig/fontconfig.h>

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_LCD_FILTER_H

typedef enum 
{
  ORD_GRAY,
  ORD_RGB,
  ORD_BGR
} sp_ord_t;

typedef struct
{
  unsigned char **data;
  int width;
  int height;

  /* current font */
  FT_Library library;
  FT_Face face;
  int grayscale; /* 0.0 to 1.0 */
  int text_direction;
  const char *script;
  uint32_t script_tag;
  const char *lang;
  FT_Int32 load_flags;
  FT_Render_Mode render_mode;
  sp_ord_t ord;
} bitmap_t;

/* pxsize == 0 -> don't initialize face */
char *freetype_version(char *string, int maxlen);
int ft_initialize_bitmap(bitmap_t *bitmap, int height, int width);
int ft_bitmap_set_font(bitmap_t *bitmap, 
                       FcPattern *pattern,
                       int pxsize,
                       int grayscale,
                       int text_direction, 
                       const char *script, 
                       const char *lang);
int ft_reduce_height(bitmap_t *bitmap, int newheight);
void ft_free_bitmap(bitmap_t *bitmap);

int ft_text_length(uint32_t text[], FcPattern *pattern, int pxsize,
                   int dir, const char *script, const char *lang);
int ft_draw_text(uint32_t text[], int x, int y, bitmap_t *bitmap);

void ft_fill_region(bitmap_t *bitmap, int left, int top, 
                    int right, int bottom, unsigned char gray);
int ft_rot270(bitmap_t *bitmap);
#endif
