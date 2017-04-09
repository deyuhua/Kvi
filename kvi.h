#ifndef KVI_H
#define KVI_H
#include <stdio.h>
#include <termios.h>
#include <limits.h>
#include <assert.h>

/***********************************************
 *                 MACRO
 ************************************************/
/* define bool type */
#define true 1
#define false 0

/* soft tab size */
#define STAB_SIZE 4
#define INIT_SIZE 512

/***********************************************
 *                data structure
 ************************************************/
/* cache data structure*/
typedef struct cache {
  char *begin, *cur, *end;
  int colidx, rowidx;
  int size;
} cache;


/* file cache list */
typedef struct frame {
  char *filename;
  char *extension;
  void *history;
  int is_dirty;
  void *syntax_highlight;
  cache *cache;
  struct frame *next;
} frame;


/* kvi mode */
typedef enum mode {
  INS = 0, // just edit cache
  CMD, // just exec commands
  QUIT // exit kvi editor
} mode;


typedef struct _global global;


/* command table */
typedef struct cmdnode {
  int key;
  char *cmd_name;
  void (*func)(global *);
} cmdnode;


/* define special key */
enum KeyCode { KEYEND = -20, UNKNOWN=-6,
               LEFT, RIGHT, DOWN, UP, ESC };


/* global variable at hear */
struct _global {
  mode mode;   // edit mode
  frame head;  // frame header
  struct termios old;  // old termios setting
  cmdnode ins_mode_table[-KEYEND + UCHAR_MAX + 8]; // ins mode table
  cmdnode cmd_mode_table[-KEYEND + UCHAR_MAX + 8]; // cmd mode table
  frame *cur_frame; // cur frame on edit
  int key;
};


enum TestAttr{AllOff=0, BoldOn, Underscore, BlinkOn};
enum FgColor{FgBlack=30, FgRed, FgGreen, FgYellow, FgBlue, FgMagenta, FgCyan, FgWhite};
enum BgColor{BgBlack=40, BgRed, BgGreen, BgYellow, BgBlue, BgMagenta, BgCyan, BgWhite};

#define RESET   "\x1b[0m"


/***********************************************
 *           function signature
 ************************************************/
// initial global kVI
global *init_global();
int get_key();
void pretty_print(cache *c);

// memory opeartion
void *realloc_cache(cache *);
cache *create_cache();
frame *add_frame(global *, char *);
void free_frames(global *);
void copy_forward(char *, char *, int);
void copy_backward(char *, char *, int);
void read_file2frame(frame *);
void write_frame2file(frame *);

// corsor move
void justify_corsor(cache *);

// cmd realte
int dispath_cmd(global *);
void register_all(global *);

/***********************************************
 *           func macro
 ************************************************/
#define clear() (system("clear"))
#define saft_free(p) do                         \
    {                                           \
      if ((p)) {                                \
        free((p));                              \
        p = NULL;                               \
      }                                         \
    } while(0)

#define saft_free_cache(c)                      \
  saft_free((c)->begin);                        \
  saft_free(c)

#define saft_malloc(p, size, clear, ret)        \
  if (((p) = malloc((size))) == NULL) {         \
    saft_free((clear));                         \
    return (ret);                               \
  }

#define insert_cur(c, key)                                    \
  if ((c)->cur == (c)->end) {                                 \
    *(c)->cur++ = key;                                        \
    (c)->end++;                                               \
    break;                                                    \
  }                                                           \
  copy_backward((c)->cur, (c)->cur + 1, (c)->end - (c)->cur); \
  *((c)->cur++) = (key);                                      \
  (c)->end++

#define insert_soft_tab(c)                                            \
  copy_backward((c)->cur, (c)->cur + STAB_SIZE, (c)->end - (c)->cur); \
  memset((c)->cur, ' ', STAB_SIZE);                                   \
  (c)->cur += STAB_SIZE;                                              \
  (c)->end += STAB_SIZE;

#define delete_cur(c)                                                 \
  if ((c)->cur == (c)->begin)                                         \
    break;                                                            \
  copy_forward((c)->cur, (c)->cur - 1, ((c)->end--) - ((c)->cur)--);  \
  if ((c)->cur >= (c)->end) {                                         \
    (c)->cur = (c)->end - 1;                                          \
  }

#define print_word(buf, cs)                     \
  fprintf(stdout, "%s", (buf));                 \
  (cs) = (buf);                                 \
  memset((buf), '\0', INIT_SIZE)

#define not_overflow(buf, cs) (cs) - (buf) < INIT_SIZE
#define print_line(line, color) fprintf(stdout, "\x1b[%dm%-4d" RESET, (color), (line))

/* register  */
#define register2mode(kVI, key, cmd, name, m)             \
  assert(register_cmd((kVI), (key), (cmd), (name), (m)))

#define register2ins(kVI, key, cmd, name)        \
  register2mode((kVI), (key), (cmd), (name), INS)

#define register2cmd(kVI, key, cmd, name)        \
  register2mode((kVI), (key), (cmd), (name), CMD)

#define ensure_corsor(c) \
  (c)->cur = (c->cur) < (c->begin) ? (c)->begin : \
    ((c)->cur > (c)->end ? (c)->end : (c)->cur)

#endif
