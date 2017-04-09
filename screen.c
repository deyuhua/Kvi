#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include "kvi.h"


/***********************************************
 *              pretty print
 ************************************************/
void pretty_print(cache *c) {

  int linenum = 1;
  char buf[INIT_SIZE] = {'\0'};
  char *cs = buf; // cs means corsor

  clear();
  print_line(linenum, FgGreen);

  for(char *p = c->begin; p <= c->end; p++) {

    if (isalpha(*p) && (not_overflow(buf, cs))) {

      *cs++ = *p;
      continue;
    }

    print_word(buf, cs);

    if (*p == '\n') {

      putchar(*p);
      print_line(++linenum, FgGreen);
    } else {

      putchar(*p);
    }
  }

  justify_corsor(c);
  fprintf(stdout, "\n\033[%d;%dH", c->rowidx + 1, c->colidx + 5);
  fflush(stdout);
}

