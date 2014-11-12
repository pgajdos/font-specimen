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

#include <stdio.h>

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_OPENTYPE_VALIDATE_H
#include FT_TRUETYPE_TABLES_H
#include FT_GLYPH_H

#include "ft.h"
#include "hbz.h"
#include "fc.h"
#include "error.h"

char *freetype_version(char *string, int maxlen)
{
  int major, minor, patch;
  FT_Library library;
  if (FT_Init_FreeType(&library))
  {
    font_specimen_error("freetype: can not initialize library");
    return NULL;
  }
  FT_Library_Version(library, &major, &minor, &patch);
  FT_Done_FreeType(library);

  snprintf(string, maxlen, "%d.%d.%d", major, minor, patch);
  return string;
}

int ft_initialize_bitmap(bitmap_t *bitmap, int height, int width)
{
  int row;

  FT_Error err;

  bitmap->data = (unsigned char **)malloc(height*sizeof(unsigned char *));
  if (! bitmap->data)
  {
    font_specimen_error("freetype: no free memory");
    return -1;
  }

  for (row = 0; row < height; row++)
  {
    bitmap->data[row] = (unsigned char *)malloc(width*sizeof(unsigned char));
    if (! bitmap->data[row])
    {
      font_specimen_error("freetype: out of memory");
      return -1;
    }
    memset(bitmap->data[row], 255, width*sizeof(unsigned char)); /* white */
  }

  bitmap->height = height;
  bitmap->width = width;

  err = FT_Init_FreeType(&bitmap->library);
  if (err)
  {
    font_specimen_error("freetype: can not initialize library");
    return -1;
  }

  bitmap->face = NULL;

  return 0;
}

int ft_bitmap_set_font(bitmap_t *bitmap, 
                       FcPattern *pattern, 
                       int pxsize,
                       int grayscale,
                       int text_direction,
                       const char *script, 
                       const char *lang)
{
  FcBool scalable;
  FcBool hinting;
  FcBool autohint;
  FcBool antialias;
  int hintstyle;
  FcBool embeddedbitmaps;
  int subpixel_layout;
  int lcdfilter;

  const char *family;
  const char *style;
  const char *file; 
  const char *fontformat;
  double size, real_size;

  FT_Error err;
  FcPattern *font, *p;

  if (bitmap->face)
    FT_Done_Face(bitmap->face);

  if (fontconfig_pattern_get_string(pattern, FC_FILE, &file) < 0)
    return -1;

  if (! pxsize)
  {
    if (fontconfig_pattern_get_double(pattern, FC_PIXEL_SIZE, &size) < 0)
      return -1;
    pxsize = size;
  }

  if (fontconfig_pattern_get_string(pattern, FC_FONTFORMAT, &fontformat) < 0)
    return -1;
  if (strcmp(fontformat, "PCF") == 0)
  {
    /* let fontconfig return nearest size for given size and
       bitmap font */
    if (fontconfig_pattern_get_string(pattern, FC_FAMILY, &family) < 0)
      return -1;
    if (fontconfig_pattern_get_string(pattern, FC_STYLE, &style) < 0)
      return -1;

    if (!(p = fontconfig_pattern_new()))
      return -1;
    if (fontconfig_pattern_set_string(p, FC_FAMILY, family) < 0)
      return -1;
    if (fontconfig_pattern_set_string(p, FC_STYLE, style) < 0)
      return -1;
    if (fontconfig_pattern_set_double(p, FC_SIZE, (double)pxsize) < 0)
      return -1;
    if (! (font = fontconfig_get_font(p)))
      return -1;
    fontconfig_pattern_destroy(p);

    if (fontconfig_pattern_get_double(font, FC_PIXEL_SIZE, &real_size) < 0)
      return -1;
    if (fontconfig_pattern_set_double(pattern, FC_PIXEL_SIZE, real_size) < 0)
      return -1;

    pxsize = (int)real_size;

    if (fontconfig_pattern_get_string(font, FC_FILE, &file) < 0)
      return -1;
    if (fontconfig_pattern_set_string(pattern, FC_FILE, file) < 0)
      return -1;
  }

  bitmap->grayscale = grayscale;

  err = FT_New_Face(bitmap->library, (char *)file, 0, &bitmap->face);
  if (err)
  {
    font_specimen_error("freetype: can not create face object");
    return -1;
  }

  err = FT_Set_Pixel_Sizes(bitmap->face, 0, pxsize);
  if (err)
  {
    font_specimen_error("freetype: can not set face size");
    return -1;
  }

  if (fontconfig_pattern_get_bool(pattern, FC_SCALABLE, &scalable) < 0)
    return -1;

  bitmap->load_flags = FT_LOAD_DEFAULT;
  bitmap->render_mode = FT_RENDER_MODE_NORMAL;

  if (scalable)
  {
    if (fontconfig_pattern_get_bool(pattern, FC_ANTIALIAS, &antialias) < 0 ||
        fontconfig_pattern_get_bool(pattern, FC_HINTING, &hinting) < 0 ||
        fontconfig_pattern_get_bool(pattern, FC_AUTOHINT, &autohint) < 0 ||
        fontconfig_pattern_get_integer(pattern, FC_HINT_STYLE, &hintstyle) < 0 ||
        fontconfig_pattern_get_bool(pattern, FC_EMBEDDED_BITMAP, &embeddedbitmaps) < 0 ||
        fontconfig_pattern_get_integer(pattern, FC_RGBA, &subpixel_layout) < 0 ||
        fontconfig_pattern_get_integer(pattern, FC_LCD_FILTER, &lcdfilter) < 0)
      return -1;

    if (!antialias)
    {
      bitmap->render_mode = FT_RENDER_MODE_MONO;
      bitmap->load_flags |= FT_LOAD_TARGET_MONO;
    }

    if (!hinting)
      bitmap->load_flags |= FT_LOAD_NO_HINTING;

    if (autohint)
      bitmap->load_flags |= FT_LOAD_FORCE_AUTOHINT;

    switch(hintstyle)
    {
      case FC_HINT_NONE:   bitmap->load_flags |= FT_LOAD_NO_HINTING; break;
      case FC_HINT_SLIGHT: bitmap->load_flags |= FT_LOAD_TARGET_LIGHT; break;
      case FC_HINT_MEDIUM: bitmap->load_flags |= FT_LOAD_TARGET_NORMAL; break;
      case FC_HINT_FULL:   bitmap->load_flags |= FT_LOAD_TARGET_NORMAL; break;
    }

    if (!embeddedbitmaps)
      bitmap->load_flags |= FT_LOAD_NO_BITMAP;
  }

  bitmap->text_direction = text_direction;
  bitmap->script = script;
  bitmap->lang = lang;

  if (text_direction >= 2)
    bitmap->load_flags |= FT_LOAD_VERTICAL_LAYOUT;

  bitmap->ord = ORD_GRAY;
  if (0 < subpixel_layout && subpixel_layout < 5)
  {
    FT_Library_SetLcdFilter(bitmap->library, lcdfilter);

    bitmap->ord = ORD_RGB;
    if (subpixel_layout == 2 || subpixel_layout == 4)
      bitmap->ord = ORD_BGR;
  }

  if (subpixel_layout == 1 || subpixel_layout == 2)
  {
    bitmap->load_flags |= FT_LOAD_TARGET_LCD;
    bitmap->render_mode = FT_RENDER_MODE_LCD;
  }

  if (subpixel_layout == 3 || subpixel_layout == 4)
  {
    bitmap->load_flags |= FT_LOAD_TARGET_LCD_V;
    bitmap->render_mode = FT_RENDER_MODE_LCD_V;
  }

  return 0;
}

int ft_reduce_height(bitmap_t *bitmap, int new_height)
{
  int row;

  if (new_height < 0 || new_height > bitmap->height)
  {
    font_specimen_error("freetype: can not reduce height (wrong new_height)");
    return -1;
  }

  for (row = bitmap->height - 1; row > new_height - 1; row--)
    free(bitmap->data[row]);
  bitmap->height = new_height; 
  return 0;
}

void ft_free_bitmap(bitmap_t *bitmap)
{
  int row;
  for (row = 0; row < bitmap->height; row++)
    free(bitmap->data[row]);
  free(bitmap->data);

  bitmap->data = NULL;
  bitmap->height = 0;
  bitmap->width = 0;

  FT_Done_Face(bitmap->face);
  FT_Done_FreeType(bitmap->library);
  bitmap->load_flags = 0;
}

void draw_bitmap(FT_Bitmap *glyph,
                 FT_Int x,
                 FT_Int y, 
                 bitmap_t bitmap,
                 int monochrome)
{
  FT_Int i, j, p, q;
  FT_Int x_max = x + glyph->width;
  FT_Int y_max = y + glyph->rows;

  char gray;

  for (i = x, p = 0; i < x_max; i++, p++)
  {
    for (j = y, q = 0; j < y_max; j++, q++)
    {
      if (i < 0 || j < 0 ||
          i >= bitmap.width || j >= bitmap.height)
        continue;

      if (monochrome)
      {
        if (glyph->buffer[glyph->pitch * q + (p >> 3)] & (128 >> (p & 7)))
        {
          //gray = ~(255*bitmap.grayscale/100);
          //if (gray <  bitmap.data[j][i])
          //bitmap.data[j][i] = gray;
          bitmap.data[j][i] = ~(~bitmap.data[j][i] | (255*bitmap.grayscale/100));
        }
      }
      else
      {
        bitmap.data[j][i] &= ~((char)((int)glyph->buffer[q * glyph->width + p]*bitmap.grayscale/100));
      }
    }
  }
}

static int ft_draw_text_(uint32_t text[], int x,  int y,
                         bitmap_t *bitmap, FT_Bool dry);

int ft_draw_text(uint32_t text[], int x,  int y, bitmap_t *bitmap)
{
  return ft_draw_text_(text, x, y, bitmap, 0);
}

int ft_text_length(uint32_t text[], FcPattern *pattern, int pxsize,
                   int dir, const char *script, const char *lang)
{
  bitmap_t bitmap;
  int len;
  if (ft_initialize_bitmap(&bitmap, 0, 0) < 0)
    return -1;
  if (ft_bitmap_set_font(&bitmap, pattern, pxsize, 1.0,
                         dir, script, lang) < 0)
    return -1;
  len = ft_draw_text_(text, 0, 0, &bitmap, 1);
  ft_free_bitmap(&bitmap);
  return len;
}

int text_length(uint32_t text[])
{
  int len = 0;
  while (text[len])
    len++;
  return len;
}

/* dry true: do not draw the bitmap, only return string length */
static int ft_draw_text_(uint32_t text[], int x,  int y,
                  bitmap_t *bitmap, FT_Bool dry)
{
  FT_Error err;
  FT_Vector pen;

  FT_UInt *glyph_codepoints;
  FT_Vector *glyph_offsets;
  FT_Vector *glyph_advances;
  FT_Vector *glyph_positions;
  FT_Glyph *glyphs;

  int monochrome;
  int nglyphs, g;
  int sum_advances_x, sum_advances_y;
  int text_width;

  text_width = 0;
  nglyphs = hbz_glyphs(text, 
                       text_length(text), 
                       bitmap->script, 
                       bitmap->lang, 
                       bitmap->text_direction,
                       bitmap->face, 
                       &glyph_codepoints, 
                       &glyph_offsets,
                       &glyph_advances,
                       &sum_advances_x,
                       &sum_advances_y);

  if (nglyphs == -1)
    return -1;

  glyph_positions = malloc(nglyphs*sizeof(FT_Vector));
  glyphs = malloc(nglyphs*sizeof(FT_Glyph));
  if (glyph_positions == NULL || glyphs == NULL)
  {
    font_specimen_error("freetype: out of memory");
    return -1;
  }

  pen.x = x << 6;
  if (bitmap->text_direction == 1)
    pen.x -= sum_advances_x;
  pen.y = y << 6;
  if (bitmap->text_direction == 3)
    pen.y -= (-sum_advances_y);
  if (bitmap->text_direction == 2)
    pen.y -= glyph_advances[0].y - glyph_offsets[0].y;
  for (g = 0; g < nglyphs; g++)
  {
    glyph_positions[g].x = (pen.x + glyph_offsets[g].x) >> 6;
    glyph_positions[g].y = (pen.y - glyph_offsets[g].y) >> 6;
    pen.x += glyph_advances[g].x;
    pen.y -= glyph_advances[g].y;

    err = FT_Load_Glyph(bitmap->face, 
                          glyph_codepoints[g], bitmap->load_flags);
    if (err)
    {
      font_specimen_error("freetype: can not load glyph");
      return -1;
    }
 
    err = FT_Get_Glyph(bitmap->face->glyph, &glyphs[g]);
    if (err)
    {
      font_specimen_error("freetype: can not get glyph");
      return -1;
    }
  }

  /* one of operands is zero */
  text_width = (sum_advances_x + (-sum_advances_y)) >> 6;

  free(glyph_codepoints);
  free(glyph_offsets);
  free(glyph_advances);

  if (! dry)
  {
    for (g = 0; g < nglyphs; g++)
    {
      monochrome = 0;
      if (bitmap->load_flags & FT_LOAD_TARGET_MONO || 
          glyphs[g]->format == FT_GLYPH_FORMAT_BITMAP)
        monochrome = 1;

      err = FT_Glyph_To_Bitmap(&glyphs[g], bitmap->render_mode, 
                                 NULL, 1);
      if (err)
      {
        font_specimen_error("freetype: can not render glyph");
        return -1;
      }

      FT_BitmapGlyph bit;
      bit = (FT_BitmapGlyph)glyphs[g];
      draw_bitmap(&bit->bitmap, 
                  glyph_positions[g].x + bit->left, 
                  glyph_positions[g].y - bit->top, 
                  *bitmap, monochrome);
    }
  }

  free(glyph_positions);
  free(glyphs);

  if (text_width < 0)
    text_width = 0;
  return text_width;
}

void ft_fill_region(bitmap_t *bitmap, int left, int top, 
                    int right, int bottom, unsigned char gray)
{
  int i, j;

  if (left >= bitmap->width ||
      top  >= bitmap->height ||
      right < 0 ||
      bottom < 0)
    return;

  if (left < 0)
    left = 0;
  if (top < 0)
    top = 0;
  if (right >= bitmap->width)
    right = bitmap->width - 1;
  if (bottom >= bitmap->height)
    bottom = bitmap->height - 1;

  for (i = left; i <= right; i++)
    for (j = top; j <= bottom; j++)
      bitmap->data[j][i] = gray;
}

int ft_rot270(bitmap_t *bitmap)
{
  int tmp;
  int row, col;
  unsigned char **data;

  data = bitmap->data;

  tmp = bitmap->height;
  bitmap->height = bitmap->width;
  bitmap->width = tmp;

  bitmap->data 
    = (unsigned char **)malloc(bitmap->height*sizeof(unsigned char *));
  if (! bitmap->data)
  {
    font_specimen_error("freetype: no free memory");
    return -1;
  }

  for (row = 0; row < bitmap->height; row++)
  {
    bitmap->data[row]
      = (unsigned char *)malloc(bitmap->width*sizeof(unsigned char));
    if (! bitmap->data[row])
    {
      font_specimen_error("freetype: no free memory");
      return -1;
    }
  }

  for (row = 0; row < bitmap->height; row++)
    for (col = 0; col < bitmap->width; col++)
      bitmap->data[row][bitmap->width-col-1] = data[col][row];

  /* bitmap->width is now former bitmap->height */
  for (row = 0; row < bitmap->width; row++)
    free(data[row]);
  free(data);

  return 0;
}

