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
#include <unistd.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>

#include "specimen.h"

/* removes spaces and slashes from string */
void remove_spaces_and_slashes(char *string)
{
  char *p1 = string, *p2 = string;

  while (*p2 != '\0')
  {
    if (!isspace(*p2) && *p2 != '/')
    {
      *p1 = *p2;
      p1++;
    }
    else
    {
      /* a capital letter before space */
      /* as side efect of this, both 'ETL fixed' */
      /*  and 'ETL Fixed' maps to the 'ETLFixed' */
      *(p2 + 1) = toupper(*(p2 + 1));
    }
    p2++;
  }

  *p1 = '\0';
  return;
}

void usage(const char *err)
{
  if (err)
    fprintf(stderr, "ERROR: %s\n\n", err);
  fprintf(stderr, "Usage: font-specimen [-d] -p pattern [-option1 value1 [...]]\n");
  fprintf(stderr, "       font-specimen [-d] -l\n");
  fprintf(stderr, "\n");
  fprintf(stderr, "       Generates specimen for given font and\n");
  fprintf(stderr, "       writes it to PNG file.\n");
  fprintf(stderr, "\n");
  fprintf(stderr, "       -p  string:  fontconfig-like pattern including rendering options\n");
  fprintf(stderr, "                    [mandatory; otherwise usage is displayed]\n");
  fprintf(stderr, "       -s  string:  unicode script name to take the sentence from\n");
  fprintf(stderr, "                    [default value: the most coveraged script]\n");
  fprintf(stderr, "       -o  string:  name of the file\n");
  fprintf(stderr, "                    [default value: ${pattern}-${script}.png]\n");
  fprintf(stderr, "       -t  string:  type of specimen [waterfall, compact]\n");
  fprintf(stderr, "                    [default value: compact]\n");
  fprintf(stderr, "       -w  int:     width of the PNG, 0 for auto\n");
  fprintf(stderr, "                    [default value: 0]\n");
  fprintf(stderr, "       -h  int:     height of th PNG, 0 for auto\n");
  fprintf(stderr, "                    [default value: 0]\n");
  fprintf(stderr, "\n");
  fprintf(stderr, "       -l           lists significant scripts and its coverage for\n");
  fprintf(stderr, "                    given font (do not write any png)\n");
  fprintf(stderr, "       -d           print errors\n");

}

int main(int argc, char *argv[])
{
  const int maxscripts = 20;

  int opt;
  const char *pattern;
  const char *script;
  specimen_type_t type;
  int width, height;  

  int script_list;

  char pngname[FILENAME_MAX];
  const char *scripts[maxscripts];
  double coverages[maxscripts];
  int s, nscripts;
  FILE *png;

  pattern = NULL;
  script_list = 0;
  script = NULL;
  pngname[0] = '\0';
  width = height = 0;
  type = SPECIMEN_COMPACT;
  while ((opt = getopt(argc, argv, "p:s:o:lt:w:h:d")) != -1)
  {
    switch (opt)
    {
      case 'p':
        pattern = optarg;
        break;
      case 's':
        script = optarg;
        break;
      case 'o':
        snprintf(pngname, FILENAME_MAX, "%s", optarg);
        break;
      case 't':
        if (strcmp(optarg, "compact") == 0)
        {
          type = SPECIMEN_COMPACT;
        }
        else if (strcmp(optarg, "waterfall") == 0)
        {
          type = SPECIMEN_WATERFALL;
        }
        else
        {
          usage("Wrong type of specimen.");
          return 1;
        }
        break;
      case 'w':
        width = atoi(optarg);
        if (width < 0)
        {
          usage("Wrong width.");
          return 1;
        }
        break;
      case 'h':
        height = atoi(optarg);
        if (height < 0)
        {
          usage("Wrong height.");
          return 1;
        }
        break;
      case 'd':
        specimen_set_debug(1);
      case 'l':
        script_list = 1;
        break;
    }
  }

  if (!pattern)
  {
    usage(NULL);
    return 1;
  }

  nscripts = specimen_font_scripts(pattern, SCRIPT_SORT_PERCENT, 
                                   scripts, coverages, maxscripts);
  if (nscripts < 0)
  {
    fprintf(stderr, "Can not get list of scripts from the font.\n");
    return 1;
  }

  if (script_list)
  {
    if (nscripts == 0)
    {
      fprintf(stderr, "No valid scripts detected in the font.\n");
      return 1;
    }

    for (s = 0; s < nscripts; s++)
    {
      fprintf(stdout, "%s (%.1f)\n",
              scripts[s], coverages[s]);
    }
  
    return 0;
  }

  if (!script)
  {
    if (nscripts > 0)
      script = scripts[0];
    else
    {
      fprintf(stderr, "No script found in given font.\n");
      return 1;
    }
  }
  else
  {
    for (s = 0; s < nscripts; s++)
      if (strcmp(script, scripts[s]) == 0)
        break;
  
    if (s == nscripts)
    {
      fprintf(stderr, "Given script not found in the font.\n");
      return 1;
    }
  }


  if (pngname[0] == '\0')
  {
    snprintf(pngname, FILENAME_MAX, "%s-%s.png", pattern, script);
    remove_spaces_and_slashes(pngname);
  }

  fprintf(stderr, "Writing %s.\n", pngname);
    
  png = fopen(pngname, "wb");
  if (!png)
  {
    fprintf(stderr, "Can not open %s.\n", pngname);
    return 1;
  }

  if (specimen_write(type, pattern, script, png, width, height) < 0)
  {
    fprintf(stderr, "Can not write specimen.");
    return 1;
  }

  return 0;
}

