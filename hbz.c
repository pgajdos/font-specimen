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

#include "hbz.h"
#include "error.h"

#include <ft2build.h>
#include FT_FREETYPE_H

#include <hb.h>
#include <hb-ft.h>

static int hbz_direction(int dir)
{
  switch (dir)
  {
    case TDIR_L2R: return HB_DIRECTION_LTR;
    case TDIR_R2L: return HB_DIRECTION_RTL;
    case TDIR_T2B: return HB_DIRECTION_TTB;
    case TDIR_B2T: return HB_DIRECTION_BTT;
    default: return HB_DIRECTION_INVALID;
  }
}

const char *hbz_version(void)
{
  return HB_VERSION_STRING;
}

unsigned hbz_glyphs(uint32_t s[], 
                    int slen,
                    const char *script, 
                    const char *lang,
                    text_dir_t dir, 
                    FT_Face face,
                    FT_UInt **glyph_codepoints, 
                    FT_Vector **glyph_offsets, 
                    FT_Vector **glyph_advances,
                    int *advances_sum_x,
                    int *advances_sum_y)
{
  unsigned g, glyph_count;

  hb_font_t *hb_ft_font;
  hb_buffer_t *hb_buf;
  hb_glyph_info_t *hb_glyph_infos;
  hb_glyph_position_t *hb_glyph_positions;

  hb_ft_font = hb_ft_font_create(face, NULL);
  hb_buf = hb_buffer_create();

  hb_buffer_set_direction(hb_buf, hbz_direction(dir));
  if (script && script[0])
    hb_buffer_set_script(hb_buf, unicode_script_tag(script));
  if (lang[0])
    hb_buffer_set_language(hb_buf, 
                           hb_language_from_string(lang, strlen(lang)));

  hb_buffer_add_utf32(hb_buf, s, slen, 0, slen);
  hb_shape(hb_ft_font, hb_buf, NULL, 0);
  
  hb_glyph_infos = hb_buffer_get_glyph_infos(hb_buf, NULL);
  hb_glyph_positions = hb_buffer_get_glyph_positions(hb_buf, &glyph_count);

  if (glyph_count <= 0)
  {
    error("harfbuzz: glyph positions couldn't be figured out");
    return 1;
  }

  *glyph_codepoints = malloc(glyph_count*sizeof(FT_UInt));
  *glyph_offsets = malloc(glyph_count*sizeof(FT_Vector));
  *glyph_advances = malloc(glyph_count*sizeof(FT_Vector));
  if (*glyph_codepoints == NULL || 
      *glyph_offsets == NULL || 
      *glyph_advances == NULL)
  {
    error("harfbuzz: out of memory");
    return -1;
  }

  *advances_sum_x = *advances_sum_y = 0;
  for (g = 0; g < glyph_count; g++)
  {
    /* for bitmap font there can be no coverage of certain
       script for certain size */
    if (hb_glyph_infos[g].codepoint == 0)
    {
      error("harfbuzz: no size for this font and script");
      return -1;
    }
    (*glyph_codepoints)[g] = hb_glyph_infos[g].codepoint;
    (*glyph_offsets)[g].x = hb_glyph_positions[g].x_offset;
    (*glyph_offsets)[g].y = hb_glyph_positions[g].y_offset;
    (*glyph_advances)[g].x = hb_glyph_positions[g].x_advance;
    (*glyph_advances)[g].y = hb_glyph_positions[g].y_advance;
    *advances_sum_x += (*glyph_advances)[g].x;
    *advances_sum_y += (*glyph_advances)[g].y;
  }

  hb_buffer_destroy(hb_buf);
  hb_font_destroy(hb_ft_font);
  return glyph_count;
}

