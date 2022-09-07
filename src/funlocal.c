/*-----------------------------------------------------------------
 * Local functions
 *
 * This file is reserved for local functions that you may wish
 * to hack into PennMUSH. Read parse.h for information on adding
 * functions. This file will not be overwritten when you update
 * to a new distribution, so it's preferable to add new functions
 * here and leave the other fun*.c files alone.
 *
 */

/* Here are some includes you're likely to need or want.
 * If your functions are doing math, include <math.h>, too.
 */

#include "copyrite.h"

#include <string.h>

#include "attrib.h"
#include "command.h"
#include "conf.h"
#include "dbdefs.h"
#include "externs.h"
#include "flags.h"
#include "function.h"
#include "game.h"
#include "ansi.h"
#include "lock.h"
#include "log.h"
#include "match.h"
#include "memcheck.h"
#include "markup.h"
#include "mushdb.h"
#include "mymalloc.h"
#include "parse.h"
#include "privtab.h"
#include "strutil.h"
#include "mushtype.h"

extern DESC *lookup_desc(dbref executor, const char *name);

void local_functions(void);

/* Here you can use the new add_function instead of hacking into function.c
 * Example included :)
 */

#ifdef EXAMPLE
FUNCTION(local_fun_silly) { safe_format(buff, bp, "Silly%sSilly", args[0]); }

#endif

/* Takes two arguments, returns an HTMLified coloured version of the string
 * in the second argument. The first argument consists of one or two
 * hex strings separated by a forward-slash. The first hex string is
 * the foreground colour, the second is the background colour. If
 * there is no second hex string, the background colour is black.
 * The hex strings are in the format #RRGGBB, where RR is the red
 * component, GG is the green component, and BB is the blue component.
 * The hex strings may be preceded by a hash sign (#) or not.
 */

FUNCTION(local_fun_rgbcolor)
{
  char *list[BUFFER_LEN / 2];
  char argval[BUFFER_LEN];
  char *ap;
  memset(list, 0, sizeof(list));
  memset(argval, 0, sizeof(argval));
  ap = argval;

  ansi_data colors;
  int hex = -1;
  char value[BUFFER_LEN];
  char aColor[COLOR_NAME_LEN];

  if (!define_ansi_data(&colors, remove_markup(args[0], NULL))) {
    if (HAS_ANSI(colors)) {
      hex = color_to_hex(colors.fg, (colors.bits & 1));
      snprintf(aColor, COLOR_NAME_LEN, "#%06x", hex);
    }
  }

  strcpy(value, args[1]);

  int res =
    list2arr(list, BUFFER_LEN / 2, remove_markup(args[0], NULL), ' ', 1);
  if (res == 2) {
    safe_format(argval, &ap, "color=%s bgcolor=%s", list[0], list[1]);
  } else if (res == 1) {
    safe_format(argval, &ap, "color=%s bgcolor=%s",
                (hex > 0 ? aColor : list[0]), "#000000");
  } else {
    safe_str("#-1", buff, bp);
    return;
  }

  safe_tag_wrap("font", argval, value, buff, bp, executor);
  *ap = '\0';
  memset(list, 0, sizeof(list));
  // mush_free(list,"ptarray");
}

  void local_functions(void)
  {
    function_add("RGB", local_fun_rgbcolor, 2, 2, FN_REG);
#ifdef EXAMPLE
    function_add("SILLY", local_fun_silly, 1, 1, FN_REG);
#endif
  }
