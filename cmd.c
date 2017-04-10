#include <string.h>
#include <ctype.h>
#include "kvi.h"

/***********************************************
 *              corsor move
 ************************************************/
cmdnode *chose_cmd_table(global *kVI, int key, mode m) {

  cmdnode *cmd_table = NULL, *slot = NULL;

  switch(m) {

  case INS:
    cmd_table = kVI->ins_mode_table;
    break;

  case CMD:
    cmd_table = kVI->cmd_mode_table;
    break;

  default:
    return NULL;
  }

  if (key < UCHAR_MAX) {

    slot = cmd_table + (-KEYEND + key);
  }

  return slot;
}


int register_cmd(global *kVI, int key, void (*cmd)(global *),
                 char *cmd_name, mode m) {

  if (!cmd_name) {
    cmd_name = "";
  }

  if (cmd) {

    cmdnode *slot = chose_cmd_table(kVI, key,  m);

    if (!slot)
      return false;

    slot->key = key;
    slot->cmd_name = cmd_name;
    slot->func = cmd;

    return true;
  }

  return false;
}


int dispath_cmd(global *kVI) {

  /* @TODO key range check */
  /* cmdnode *cmd_table = chose_cmd_table(kVI, kVI->mode); */
  cmdnode *slot = chose_cmd_table(kVI, kVI->key, kVI->mode);

  if (slot && slot->func) {

    (*slot->func)(kVI);

    return true;
  }

  return false;
}


/***********************************************
 *              corsor move cmd
 ************************************************/
void justify_corsor(cache *c) {

  c->colidx = c->rowidx = 0;
  for (char *p = c->begin; p < c->cur; p++) {

    c->colidx ++;

    if (*p == '\n') {
      c->rowidx++;
      c->colidx = 0;
    }
  }
}


void move_corsor_LR(cache *c, int step) {

  char *fin = c->cur + step;

  if (fin >= c->begin && fin <= c->end) {
    c->cur = fin;
  }

  ensure_corsor(c);

  justify_corsor(c);
}


void move_corsor_UD(cache *c, int step) {

  int count = step < 0 ? -step + 1 : step;
  int colidx = c->colidx;

  char *p = c->cur;

  if (*p == '\n') {

    if (step > 0)
      count = 0;
    else
      p += step;
  }

  for (int i = 0; p >= c->begin && p <= c->end; p += step) {

    if (*p == '\n') {
      i++;
    }

    if (i >= count) {
      break;
    }
  }


  for (++p; p <= c->end && p >= c->begin && colidx > 0 && *p != '\n'; --colidx, ++p);

  c->cur = p;
  ensure_corsor(c);

  justify_corsor(c);
}


/* MOVE RIGHT */
void corsor_move_right(global *kVI) {
  move_corsor_LR(kVI->cur_frame->cache, 1);
  
}

/* MOVE LEFT */
void corsor_move_left(global *kVI) {
  move_corsor_LR(kVI->cur_frame->cache, -1);
}

/* MOVE UP */
void corsor_move_up(global *kVI) {
  move_corsor_UD(kVI->cur_frame->cache, -1);
}

/* MOVE DOWN */
void corsor_move_down(global *kVI) {
  move_corsor_UD(kVI->cur_frame->cache, 1);
}

/* MOVE TO BEGIN OF LINE */
void corsor_move_begin(global *kVI) {

  cache *c = kVI->cur_frame->cache;
  char *begin = c->cur;

  if (*begin == '\n' && begin - 1 >= c->begin)
    begin --;

  for (; begin >= c->begin && *begin != '\n'; begin--);

  c->cur = begin == c->begin ? c->begin : begin + 1;
}

/* MOVE TO END OF LEIN*/
void corsor_move_end(global *kVI) {

  cache *c = kVI->cur_frame->cache;
  char *end = c->cur;

  for (; end <=c->end && *end != '\n'; end++);

  c->cur = end;
}

/* MOVE FORWARD */
void corsor_move_word_forward(global *kVI) {

  cache *c = kVI->cur_frame->cache;
  char *word = c->cur;

  if (!isalpha(*word)) {
    for (; word <= c->end && !isalpha(*word); word++);
  }

  for (; word <= c->end && isalpha(*word); word++);

  c->cur = word;
}

/* MOVE BACKWARD */
void corsor_move_word_backward(global *kVI) {

  cache *c = kVI->cur_frame->cache;
  char *word = c->cur;

  if (isalpha(*word) && !isalpha(*(word - 1))) {
    word--;
  }

  if (!isalpha(*word)) {
    for (; word >= c->begin && !isalpha(*word); word--);
  }

  for (; word >= c->begin && isalpha(*word); word--);

  c->cur = word == c->begin ? c->begin : word + 1;
}

/***********************************************
 *              insert char cmd
 ************************************************/
void insert_character(global *kVI) {

  cache *c = kVI->cur_frame->cache;
  int add_key = KEYEND;

  switch(kVI->key) {

  case '\t':
    insert_soft_tab(c);
    break;

  default:
    insert_cur(c, kVI->key);

    switch(kVI->key) {

    case '(':
      add_key = ')';
      break;

    case '[':
      add_key = ']';
      break;

    case '{':
      add_key = '}';
      break;

    case '<':
      add_key = '>';
      break;

    case '\'':
      add_key = '\'';
      break;

    case '\"':
      add_key = '\"';
      break;

    default:
      break;
    }

    if (add_key != KEYEND) {
      insert_cur(c, add_key);
      -- c->cur;
      ensure_corsor(c);
    }
    break;
  }

  kVI->cur_frame->is_dirty = true;
}


void delete_character(global *kVI) {

  cache *c = kVI->cur_frame->cache;
  int cur_key = *(c->cur - 1 >= c->begin ? c->cur - 1 : c->begin);

  // a break used in macro, @TODO
  do {
    delete_cur(c);
  }while(0);

  switch(cur_key) {

  case '(':
  case '[':
  case '{':
  case '<':
  case '\'':
  case '\"':
    c->cur++;

    delete_cur(c);
    ensure_corsor(c);
    break;

  default:
    break;
  }


  kVI->cur_frame->is_dirty = true;
}


void replace_character_at_corsor(global *kVI) {

  kVI->key = get_key();
  *(kVI->cur_frame->cache->cur) = kVI->key;
}


/***********************************************
 *              mode switcher cmd
 ************************************************/
void switch2cmd(global *kVI) {
  kVI->mode = CMD;
}

void switch2ins(global *kVI) {
  kVI->mode = INS;
}

void switch2quit(global *kVI) {
  kVI->mode = QUIT;
}


/***********************************************
 *             read/write cmd
 ************************************************/
void read_file2frame(frame *f) {

  FILE *fd = fopen(f->filename, "r");
  cache *c = f->cache;

  if (!fd) return;

  int ch;
  while ((ch = fgetc(fd)) != EOF) {

    if (c->end - c->begin >= c->size) {
      realloc_cache(c);
    }

    *(c->cur++) = ch;
  }

  c->end = c->cur;
  c->cur = c->begin;

  fclose(fd);
}


void write_frame2file(frame *f) {

  if (!f->is_dirty)
    return;

  FILE *fd = fopen(f->filename, "w");
  cache *c = f->cache;

  if (!fd)
    return;

  for (char *p = c->begin; p <= c->end; ) {
    fputc(*p++, fd);
  }

  fputc('\n', fd);

  fclose(fd);
}


void write(global *kVI) {

  write_frame2file(kVI->cur_frame);
}

/***********************************************
 *             delete multi line
 ************************************************/
void delete_word(cache *c) {
  char *w_end;

  for (w_end = c->cur; w_end <= c->end && isalpha(*(w_end)); w_end++);

  copy_forward(w_end, c->cur, c->end - w_end);
}


void delete_cur_line(cache *c) {

  char *begin = c->cur, *end = c->cur;

  if (*(c->cur) == '\n' && *(c->cur - 1) == '\n') {
    do {
      delete_cur(c);
    }while(0);

    c->cur = c->cur == c->end ? c->end : c->cur + 1;
    return;
  }

  for (; begin >= c->begin && *begin != '\n'; begin--);
  begin = begin == c->begin ? c->begin : begin + 1;

  for (; end <= c->end && *end != '\n'; end ++);
  end = end == c->end ? c->end : end + 1;

  copy_forward(end, begin, c->end - end);
  c->end = c->end - (end - begin - 1);
}


void delete_multi(global *kVI) {

  int key = get_key();
  cache *c = kVI->cur_frame->cache;

  switch(key) {
  case 'w':
    delete_word(c);
    break;

  case 'd':
    delete_cur_line(c);
    break;

  default:
    break;
  }

}


/***********************************************
 *             register all cmd
 ************************************************/
void register_all(global *kVI) {

  /***********************************************
   *           register INS mode cmd
   ************************************************/
  /* corsor move */
  register2ins(kVI, RIGHT, &corsor_move_right, "move right");
  register2ins(kVI, LEFT, &corsor_move_left, "move left");
  register2ins(kVI, UP, &corsor_move_up, "move up");
  register2ins(kVI, DOWN, &corsor_move_down, "move down");

  /* insert printable character */
  for (int i = 0; i < UCHAR_MAX; i++) {

    /* register all character */
    register2ins(kVI, i, &insert_character, "");
  }

  /* delete character cmd */
  register2ins(kVI, '\x7f', &delete_character, "");
  register2ins(kVI, '\b', &delete_character, "");

  /* mode switch */
  register2ins(kVI, ESC, &switch2cmd, "switch CMD mode");


  /***********************************************
   *           register CMD mode cmd
   ************************************************/
  register2cmd(kVI, 'i', &switch2ins, "i: switch to ins mode");

  register2cmd(kVI, 'h', &corsor_move_left, "h: move left");
  register2cmd(kVI, 'l', &corsor_move_right, "l: move right");
  register2cmd(kVI, 'j', &corsor_move_up, "j: move up");
  register2cmd(kVI, 'k', &corsor_move_down, "");
  register2cmd(kVI, LEFT, &corsor_move_left, "");
  register2cmd(kVI, RIGHT, &corsor_move_right, "");
  register2cmd(kVI, UP, &corsor_move_up, "");
  register2cmd(kVI, DOWN, &corsor_move_down, "");
  register2cmd(kVI, 'a', &corsor_move_begin, "a: move to begin");
  register2cmd(kVI, 'e', &corsor_move_end, "a: move to end");
  register2cmd(kVI, 'f', &corsor_move_word_forward, "f: move forward");
  register2cmd(kVI, 'b', &corsor_move_word_backward, "b: move backward");

  register2cmd(kVI, 'x', &delete_character, "x: delete character");
  register2cmd(kVI, 'q', &switch2quit, "q: quit editor");
  register2cmd(kVI, 'r', &replace_character_at_corsor, "r: replace character");
  register2cmd(kVI, 'w', &write, "w: write to file");

  register2cmd(kVI, 'd', &delete_multi, "d: delete multi");
}
