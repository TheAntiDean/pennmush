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


void local_functions(void);

/* Here you can use the new add_function instead of hacking into function.c
 * Example included :)
 */

#ifdef EXAMPLE
FUNCTION(local_fun_silly) { safe_format(buff, bp, "Silly%sSilly", args[0]); }

#endif



FUNCTION(local_fun_rgbcolor)
{
  char **list = mush_calloc(BUFFER_LEN/2, sizeof(char *), "ptrarray");
  char argval[BUFFER_LEN];
  char *ap;
  ap = argval;

  int res = list2arr(list,BUFFER_LEN /2, args[0], ' ', 1);
  if(res== 2)
  {
   safe_str("color=", argval, &ap);
   safe_str(list[0], argval, &ap);
  safe_str(" bgcolor=", argval, &ap);
   safe_str(list[1], argval, &ap);
  }
  else if(res == 1) {
    safe_str("color=", argval, &ap);
   safe_str(list[0], argval, &ap);
   safe_str("bg color=#", argval, &ap);
  }
  else
  {
    safe_str("#-1", buff, bp);
    return;
  }

      safe_tag_wrap("font", argval, args[1], buff, bp, executor);
  *ap = '\0';
  mush_free(list,"ptarray");


}


FUNCTION(local_fun_nameformat)
{
  dbref it = match_thing(executor, args[0]);
  char tbuf1[BUFFER_LEN];


  if (GoodObject(it)) {
    /* You must either be see_all, control it, or be inside it */
    // if (!(controls(executor, it) || See_All(executor) ||
    //       (Location(executor) == it))) {
    //   safe_str(T(e_perm), buff, bp);
    //   return;
    // }
    
    if (flaglist_check_long("FLAG", executor, it, "HALT", 1) == 1)
      safe_str(accented_name(it), buff, bp);
    else if (nameformat(executor, it, tbuf1,
        IsExit(it) ? shortname(it) : (char *) accented_name(it), 1,
        pe_info))
      safe_str(tbuf1, buff, bp);
    else if (IsExit(it))
      safe_str(shortname(it), buff, bp);

    else
      safe_str(accented_name(it), buff, bp);
  } else
    safe_str(T(e_notvis), buff, bp);
}

void
local_functions(void)
{
  function_add("NAMEFORMAT", local_fun_nameformat, 1, 1, FN_REG| FN_STRIPANSI);
  function_add("RGB", local_fun_rgbcolor, 2, 2, FN_REG| FN_STRIPANSI);
#ifdef EXAMPLE
  function_add("SILLY", local_fun_silly, 1, 1, FN_REG);
#endif
}
