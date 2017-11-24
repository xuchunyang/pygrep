/* Convert Pinyin into RegExp and search with it.

   Copyright (C) 2017 Chunyang Xu

   pygrep is free software; you can redistribute it and/or modify it
   under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3, or (at your option)
   any later version.

   pygrep is distributed in the hope that it will be useful, but WITHOUT
   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
   or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public
   License for more details.

   You should have received a copy of the GNU General Public License
   along with pygrep.  If not, see http://www.gnu.org/licenses.
*/

#define PCRE2_CODE_UNIT_WIDTH 8

#include <pcre2.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "sds.h"

#ifdef DEBUG
#undef DEBUG
#define DEBUG(...) do {                         \
    fprintf (stderr, "[DEBUG] ");               \
    fprintf (stderr, __VA_ARGS__);              \
    fprintf (stderr, "\n");                     \
  } while (0)
#else
#define DEBUG(...)
#endif

extern char *pinyin[];
extern char *pinyin_map[];

void exit_with_pcre2_error (char *fname, int errorcode);

int
find_pinyin (char *py)
{
  for (int i = 0; pinyin[i] != NULL; i++)
    if (strcmp (py, pinyin[i]) == 0)
      return i;
  return -1;
}

int
main (int argc, char *argv[])
{
 if (argc < 3)
    {
      fprintf (stderr, "usage: %s Pinyin... FILE\n", argv[0]);
      exit (EXIT_FAILURE);
    }

  sds pattern = sdsempty();
  char *filename = argv[argc - 1];
  pcre2_code *code = NULL;
  int errorcode;
  PCRE2_SIZE erroroffset;
  pcre2_match_data *match_data = NULL;
  int rc;
  FILE *file;
  char *line;
  size_t linecap = 512;
  ssize_t linelen;

  for (int i = 1; i < argc - 1; i++)
    {
      int j = find_pinyin (argv[i]);
      if (j < 0)
        {
          printf ("%s is not valid Pinyin\n", argv[i]);
          exit (EXIT_FAILURE);
        }
      else
          pattern = sdscat (pattern, pinyin_map[j]);
    }

  DEBUG ("regex: %.*s", (int) sdslen (pattern), pattern);

  code = pcre2_compile ((PCRE2_SPTR8) pattern,
                        (int) sdslen (pattern),
                        PCRE2_UTF,
                        &errorcode,
                        &erroroffset,
                        NULL);
  sdsfree (pattern);
  if (code == NULL)
    exit_with_pcre2_error ("pcre2_compile", errorcode);

  match_data = pcre2_match_data_create_from_pattern (code, NULL);
  if (match_data == NULL)
    {
      fprintf (stderr, "pcre2_match_data_create_from_pattern failed\n");
      exit (EXIT_FAILURE);
    }

  file = fopen (filename, "r");
  if (file == NULL)
    {
      perror ("fopen");
      exit (EXIT_FAILURE);
    }

  line = malloc (linecap);
  if (line == NULL)
    {
      perror ("malloc");
      exit (EXIT_FAILURE);
    }

  while ((linelen = getline (&line, &linecap, file)) > 0)
    {
      rc = pcre2_match (code, (PCRE2_SPTR) line, linelen, 0, 0, match_data, NULL);
      if (rc > 0)
        fwrite (line, linelen, 1, stdout);
      else if (rc < 0 && rc != PCRE2_ERROR_NOMATCH)
        exit_with_pcre2_error ("pcre2_match", rc);
    }

  free (line);
  pcre2_code_free (code);
  pcre2_match_data_free (match_data);
  return 0;
}

void
exit_with_pcre2_error (char *fname, int errorcode)
{
  PCRE2_UCHAR buffer[120];
  pcre2_get_error_message (errorcode, buffer, 120);
  fprintf (stderr, "%s failed: %s\n", fname, buffer);
  exit (EXIT_FAILURE);
}
