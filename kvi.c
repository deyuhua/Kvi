#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <limits.h>
#include "kvi.h"

/***********************************************
 *             initial global
 ************************************************/
global* init_global() {

  global *kVI;

  saft_malloc(kVI, sizeof(global), kVI, NULL);
  kVI = (global *)kVI;

  /* (global *)malloc(sizeof(global)); */
  memset(kVI, 0, sizeof(global));

  printf("%d\n", UCHAR_MAX);

  // default mode when enter editor
  kVI->mode = INS;

  kVI->head.next = NULL;
  kVI->cur_frame = NULL;

  // register all cmd at initial
  register_all(kVI);

  return kVI;
}


/***********************************************
 *           termios setting
 ************************************************/
void termios_start(global *kVI) {

  clear();
  tcgetattr(STDIN_FILENO, &kVI->old);

  // enable termios
  struct termios newt = kVI->old;

  newt.c_lflag &= ~(ECHO | ICANON);
  newt.c_cc[VMIN] = (cc_t) 0;
  tcsetattr(STDIN_FILENO, TCSANOW, &newt);

  fflush(stdout);
}


void termios_end(global *kVI) {

  clear();
  tcsetattr(STDIN_FILENO, TCSANOW, &kVI->old);

}
/***********************************************
 *           handle user input
 ************************************************/
int parse_key(char *buf) {

  if (buf[0] != '\x1b')
    return buf[0];


  if (buf[1] == '\0') {

    return ESC;

  } else if (buf[1] == '['){

    if (buf[2] >= 'A' && buf[2] <= 'D') {

      return -(buf[2] - 'A' + 2);
    }
  }

  return UNKNOWN;
}


int get_key() {

  size_t n = 0;
  char buf[3] = {'\0'}, c = '\n';

  while(n < 1) {
    n = read(fileno(stdin), &c, 1);
  }

  for (int i = 0; i < 3 && n > 0; i++) {
    buf[i] = c;
    n = read(fileno(stdin), &c, 1);
  }

  return parse_key(buf);
}


/***********************************************
 *              main loop
 ************************************************/
int main(int argc, char **argv) {

  global *kVI;
  char *filename = argc > 1 ? argv[1] : "new_file";

  kVI = init_global();

  if (!kVI) {

    fprintf(stdout, "Memory Alloc Failed!\n");
    exit(1);
  }

  termios_start(kVI);

  kVI->cur_frame = add_frame(kVI, filename);
  read_file2frame(kVI->cur_frame);

  while(kVI->cur_frame && kVI->mode != QUIT) {
    // refresh screen
    pretty_print(kVI->cur_frame->cache);

    // read from user
    kVI->key = get_key();

    // dispath cmd by input
    dispath_cmd(kVI);
  }

  termios_end(kVI);

  // write all frames into file

  // free all frame first
  free_frames(kVI);
  saft_free(kVI);

  return 0;
}
