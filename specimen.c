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

#include <fontconfig/fontconfig.h>

#include "specimen.h"
#include "unicode.h"
#include "fc.h"
#include "ft.h"
#include "img_png.h"
#include "error.h"

#define max(a, b) ((a) > (b) ? (a) : (b))

#define MAX_PX_SIZE        100
#define MAX_SENTENCE_LEN    50


typedef struct
{
  int x;
  int y;
  int pxsize;
  uint32_t *sentence;
  FcPattern *pattern;
  const char *script;
  const char *lang;
  text_dir_t dir;
  int grayscale;
} specimen_string_t;

void specimen_set_debug(int on)
{
  debug = on;
}

static int strings_waterfall(uint32_t string[],
                             FcPattern *pattern, 
                             const char *script,
                             const char *lang,
                             text_dir_t dir,
                             specimen_string_t **strings,
                             int *width,
                             int *height)
{
  const int size_from = 7, size_step = 1;
  int size_to = 35;
  const int strings_dist = 7;

  int s, nsizes;
  int sumsizes;
  int i;
  int x, y, len;

  size_to = size_from + ((size_to - size_from)/size_step)*size_step;
  nsizes = (size_to - size_from)/size_step + 1;

  sumsizes = 0;
  for (s = size_from; s <= size_to; s++)
  {
    sumsizes += s;
  }

  if (nsizes <= 0 || size_to > MAX_PX_SIZE)
  {
    error("specimen: wrong size intervals");
    return -1;
  }

  *strings = (specimen_string_t*)malloc(nsizes*sizeof(specimen_string_t));
  
  if (!*strings)
  {
    error("specimen: not enough memory");
    return -1;
  }

  len = ft_text_length(string, pattern, size_to, dir, script, lang);
  if (dir < 2)
  {
    if (! *width)
      *width = len;
    if (! *height)
      *height = sumsizes + (nsizes)*strings_dist;
  }
  else
  {
    if (! *width)
      *width = sumsizes + (nsizes)*strings_dist;
    if (! *height)
      *height = len;
  }

  switch (dir)
  {
    case TDIR_L2R: /* left to right */
      x = 0;
      y = size_from + strings_dist;
      break;
    case TDIR_R2L: /* right to left */
      x = *width;
      y = size_from + strings_dist;
      break;
    case TDIR_T2B: /* top to bottom */
      x = size_from;
      y = 0;
      break;
    case TDIR_B2T: /* bottom to top */
      x = size_from;
      y = *height - strings_dist;
      break;
    default:
      return -2;
  }
 
  for (s = size_from, i = 0; i < nsizes; s += size_step, i++)
  {
    (*strings)[i].sentence = string;
    (*strings)[i].x = x;
    (*strings)[i].y = y;
    (*strings)[i].pxsize = s;
    (*strings)[i].pattern = pattern;
    (*strings)[i].script = script;
    (*strings)[i].lang = lang;
    (*strings)[i].dir = dir;
    (*strings)[i].grayscale = 100;

    if (dir < 2)
      y += s + strings_dist;
    else
      x += s + strings_dist;
  }

  return nsizes;
}

static int strings_compact(uint32_t string[],
                             FcPattern *pattern, 
                             const char *script,
                             const char *lang,
                             text_dir_t dir,
                             specimen_string_t **strings,
                             int *width,
                             int *height)
{
  const int sizes[] = {7, 8, 9, 10, 12, 15, 20, 35};
  const int shadow_size = 70;
  const int strings_dist = 10;

  int nsizes = sizeof(sizes)/sizeof(sizes[0]);
  int sumsizes;
  int i;
  int x, y, len;

  sumsizes = 0;
  for (i = 0; i < nsizes; i++)
    sumsizes += sizes[i];

  *strings = (specimen_string_t*)malloc((nsizes + 1)*sizeof(specimen_string_t));
  
  if (!*strings)
  {
    error("specimen: no memory");
    return -1;
  }

  len = ft_text_length(string, pattern, sizes[nsizes-1], dir, script, lang);
  if (dir < 2)
  {
    if (!*width)
      *width = len;
    if (!*height)
      *height = sumsizes + (nsizes + 1)*strings_dist;
  }
  else
  {
    if (!*width)
      *width = sumsizes + (nsizes + 1)*strings_dist;
    if (!*height)
      *height = len;
  }

  switch (dir)
  {
    case TDIR_L2R: /* left to right */
      x = 0;
      y = sizes[0] + strings_dist;
      break;
    case TDIR_R2L: /* right to left */
      x = *width;
      y = sizes[0] + strings_dist;
      break;
    case TDIR_T2B: /* top to bottom */
      x = sizes[0];
      y = 0;
      break;
    case TDIR_B2T: /* bottom to top */
      x = sizes[0];
      y = *height - strings_dist;
      break;
    default:
      return -2;
  }

  for (i = 0; i < nsizes; i++)
  {
    (*strings)[i].sentence = string;
    (*strings)[i].x = x;
    (*strings)[i].y = y;
    (*strings)[i].pxsize = sizes[i];
    (*strings)[i].pattern = pattern;
    (*strings)[i].script = script;
    (*strings)[i].lang = lang;
    (*strings)[i].dir = dir;
    (*strings)[i].grayscale = 100;

    if (dir < 2)
      y += sizes[i+1] + strings_dist;
    else
      x += sizes[i+1] + strings_dist;
  }

  /* gray big on the background */
  (*strings)[i].sentence = string;
  switch(dir)
  {
    case TDIR_L2R:
      (*strings)[i].x = shadow_size/2;
      (*strings)[i].y = shadow_size;
      break;
    case TDIR_R2L:
      (*strings)[i].x = *width - shadow_size/2;
      (*strings)[i].y = shadow_size;
      break;
    case TDIR_T2B:
      (*strings)[i].x = shadow_size/2;
      (*strings)[i].y = shadow_size/2;
      break;
    case TDIR_B2T:
      (*strings)[i].x = shadow_size/2;
      (*strings)[i].y = shadow_size/2;
      break;
  }

  (*strings)[i].pxsize = shadow_size;
  (*strings)[i].pattern = pattern;
  (*strings)[i].script = script;
  (*strings)[i].lang = lang;
  (*strings)[i].dir = dir;
  (*strings)[i].grayscale = 25;

  return nsizes + 1;
}

int specimen_font_scripts(const char *font,
                          script_sort_t sort,
                          const char *scripts[],
                          double coverages[],
                          int maxscripts)
{
  uinterval_stat_t *stats;
  int i, nintervals;
  FcPattern *pat, *fnt;

  pat = fontconfig_get_pattern(font);
  if (! pat)
    return -1;
  fnt = fontconfig_get_font(pat);
  if (! fnt)
    return -1;

  nintervals = unicode_interval_statistics(fnt, &stats,
                                           UI_SCRIPT, sort);
  if (nintervals < 0)
    return -1;

  for (i = 0; i < nintervals && i < maxscripts; i++)
  {
    scripts[i] = stats[i].ui_name;
    coverages[i] = stats[i].coverage;
  }

  free(stats);
  fontconfig_pattern_destroy(pat);
  fontconfig_pattern_destroy(fnt);
  return i;
}


int specimen_write(specimen_type_t type,
                    const char *font,
                    const char *script,
                    FILE *png,
                    int width,
                    int height)
{
  FcPattern *pat, *fnt;
  text_dir_t dir;
  img_transform_t transform;
  const char *lang;
  int random;
  uint32_t sentence[MAX_SENTENCE_LEN];
  int nstrings;
  int tmp;

  bitmap_t bitmap;
  int t;
  specimen_string_t *strings;

  int o, value;
  const char *rendering_options_bool[] = { FC_ANTIALIAS, 
                                           FC_HINTING, 
                                           FC_AUTOHINT, 
                                           FC_LCD_FILTER, 
                                           NULL };
  const char *rendering_options_int[] = {  FC_HINT_STYLE, 
                                           FC_RGBA, 
                                           FC_EMBEDDED_BITMAP,
                                           NULL };

  pat = fontconfig_get_pattern(font);
  if (! pat)
    return -1;
  fnt = fontconfig_get_font(pat);
  if (! fnt)
    return -1;

  /* set requested rendering options which are lost 
     via current system's fontconfig */
  o = 0;
  while (rendering_options_bool[o])
  {
    if (fontconfig_pattern_get_bool(pat, rendering_options_bool[o], &value) == 0)
      fontconfig_pattern_set_bool(fnt, rendering_options_bool[o], value);
    o++;
  }
      
  while (rendering_options_int[o])
  {
    if (fontconfig_pattern_get_bool(pat, rendering_options_int[o], &value) == 0)
      fontconfig_pattern_set_bool(fnt, rendering_options_int[o], value);
    o++;
  }
  fontconfig_pattern_destroy(pat);

  if (unicode_specimen_sentence(fnt, NULL, script, MAX_SENTENCE_LEN,
                                &dir, &transform, &lang,
                                &random, sentence) < 0)
    return -1;
  if (random == 2)
  {
    error("specimen: no symbols for this font and script");
    return -1;
  }

  switch (type)
  {
    case SPECIMEN_WATERFALL:
      nstrings = strings_waterfall(sentence, fnt, script,
                                   lang, dir, &strings,
                                   &width, &height);      
      if (nstrings < 0)
        return -1;
      break;
    case SPECIMEN_COMPACT: 
      nstrings = strings_compact(sentence, fnt, script,
                                 lang, dir, &strings,
                                 &width, &height); 
      if (nstrings < 0)
        return -1;
      break;
    default:
      return -1;
  }

  switch (transform)
  {
    case TRNS_ROT270:
      tmp = width;
      width = height;
      height = tmp;
      break;
    case TRNS_NONE:
      break;
    default:
      return -1;
  }

  if (ft_initialize_bitmap(&bitmap, height, width) < 0)
    return -1;

  for (t = 0; t < nstrings; t++)
  {
    if (ft_bitmap_set_font(&bitmap, strings[t].pattern, strings[t].pxsize,
                           strings[t].grayscale, strings[t].dir, strings[t].script, 
                           strings[t].lang) < 0)
      return -1;
    if (ft_draw_text(strings[t].sentence, strings[t].x, 
                     strings[t].y, &bitmap) < 0)
      return -1;
  }

  switch (transform)
  {
    case TRNS_ROT270:
      ft_rot270(&bitmap);
      break;
    case TRNS_NONE:
      break;
    default:
      return -1;
  }

  if (img_png_write(png, bitmap) < 0)
    return -1;

  ft_free_bitmap(&bitmap);
  free(strings);
  fontconfig_pattern_destroy(fnt);
  return 0;
}

