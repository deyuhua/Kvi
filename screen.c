#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include "kvi.h"


/***********************************************
 *              pretty print
 ************************************************/
int get_width(void) {
  struct winsize ws;
  (void) ioctl(0, TIOCGWINSZ, &ws);

  return (int) ws.ws_col;
}


int get_height(void) {
  struct winsize ws;
  (void) ioctl(0, TIOCGWINSZ, &ws);

  return (int) ws.ws_row;
}


void print_status(global *kVI) {

  char *mode = kVI->mode == INS ? "INS" : "CMD";

  fprintf(stdout, "\x1b[%d;%dm", Underscore, FgMagenta);
  fprintf(stdout, "\033[%d;%dH[%s %s]", get_height(), 0, mode, "MODE");

  fprintf(stdout, RESET);
}


void pretty_print(global *kVI) {

  int linenum = 1;
  cache *c = kVI->cur_frame->cache;
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
  print_status(kVI);

  fprintf(stdout, "\033[%d;%dH", c->rowidx + 1, c->colidx + 5);
  fflush(stdout);
}

