
/* 
   (c) COPYRIGHT URI/MIT 1999, 2000
   Please read the full copyright statement in the file COPYRIGHT.

   Authors:
      jhrg,jimg		James Gallagher (jgallagher@gso.uri.edu)
*/

/** Simple interface for loaddods error reporting. This uses two global matlab
    variables, dods_err and dods_err_msg. dods_err is a scalar and is
    initially 0 to indicate that no error has been found. When an error is
    found it is set to 1. Callers of loaddods must test this variable to
    determine if an error has occurred. If dods_err is 1 then an error
    message will be contained in dods_err_msg (a Matlab vector).

    This interface defines nine functions: err_init() is used to initialize
    the error system. The function err_msg() is used to set the dods_err
    and dods_err_msg variables. Messages are terminated by a \n character. If
    err_msg() is called more than once before the program exits, the messages
    will be appended to dods_err_msg until the size of that char array
    becomes greater than MSG_STR (4064 characters). At that point more calls
    to err_msg simply result in the `Too many messages.' string being append
    to dods_err_msg. 

    NB: It is imperative that err_init() be called by the mexFunction *every
    time it runs* since this software uses static global variables that must
    be initialized before each run of the mexFunction.

    A companion function msg() exists which can be called to send messages to
    stderr (so that users can see them right away; useful for debugging
    instrumentation and informational messages). The initialization function
    err_init() can be called so that the err_msg() function behaves
    identically to the msg() function. This provides complete backward
    compatibility with the old error reporting scheme which sent all messages
    to stderr.

    If the compatibility mode is used, callers of loaddods must be savvy and
    not look for dods_err and/or dods_err_msg since they may not exist. 

    If dods_err is false (0), the contents of dods_err_msg are undefined.

    @author jhrg */


#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <errno.h>

#include <mex.h>

#if DMALLOC
// #include "dods_memory.h"
#include "dmalloc.h"
#define mxFree(x) dods_free(x)
#define mxCalloc(x, y) dods_calloc((x), (y))
#define mxMalloc(x) dods_malloc(x)
#define mxRealloc(x, y) dods_realloc((x), (y))
#endif

#if EFENCE_MALLOC
#include "dods_memory.h"
#define mxFree(x) dods_free(x)
#define mxCalloc(x, y) dods_calloc((x), (y))
#define mxMalloc(x) dods_malloc(x)
#define mxRealloc(x, y) dods_realloc((x), (y))
#endif

#define MSG_STR 4064		/* MSG_STR + OVERFLOW = 4K */
#define OVERFLOW 32		/* We need 25, but why not make it 2^5? */

#define OVERFLOW_MSG "Too many error messages.\n"

#include "defines.h"

extern int errno;

/* While these are static, they don't behave like static members since a Mex
   Function stays resident between invocations. Thus, you MUST call
   err_init() at the start of the MexFunction. 12/3/99 jhrg */
static int _comp_mode = 0;
static int _dods_err = 0;
static char _dods_err_msg[MSG_STR+OVERFLOW] = "";
static int _overflow_dods_err_msg = 0;

/** Internal function. Glue the newest message onto other, older, messages.
    If the message limit is run over, tack a `Too many error messages'
    message on. Messages all are terminated by a newline.

    @param dem char * to the global error message cache.
    @param msg const char * to the newest message. */
static void
_append_new_msg(char *dem, const char *msg)
{
    if (_overflow_dods_err_msg)
	return;

    if (strlen(dem) + strlen(msg) > MSG_STR) {
	_overflow_dods_err_msg = 1;
	strcat(dem, OVERFLOW_MSG);
    }
    else {
	strcat(dem, msg);
    }
}	

/** Take the value of _err_state and dump it into the Matlab scalar dods_err.
 */ 
static void
_intern_dods_err(int _err_state)
{
    double *dde = mxMalloc(sizeof(double));
    double des = _err_state;	/* convert to double and store. */
    mxArray *array_ptr;

    memcpy(dde, &des, sizeof(double)); /* mxMalloc all interned memory */

    array_ptr = mxCreateDoubleMatrix(1, 1, mxREAL);
    if (!array_ptr)
	mexErrMsgTxt("Internal Error in the reporting system (1)\n\
Please report this error to support@unidata.ucar.edu");
    mxSetName(array_ptr, "dods_err");
    mxSetPr(array_ptr, dde);
    mexPutArray(array_ptr, "caller");   
}

/** Take the value of _msg and dump it into dods_err_msg. */
static void
_intern_msg(char *_msg)
{
    int error = 0;
    mxArray *pm = mxCreateString(_msg);

    mxSetName(pm, "dods_err_msg");
    error = mexPutArray(pm, "caller");
    if (error)
	mexErrMsgTxt("Internal Error in the reporting system (1)\n\
Please report this error to support@unidata.ucar.edu");
}

/** Initialize the error reporting subsystem of loaddods. 
    
    @param compat_mode If 1, use the compatibility mode where all error
    messages are printed to stderr. If 0, error messages are sent to the
    Matlab `string' variable (ML 4 compatible) and dods_err is set to 1. */
int
err_init(int compat_mode)
{
    _comp_mode = compat_mode;	/* Old behavior or new behavior? */
    if (!_comp_mode) {
	/* create/initialize the dods_err, etc. Since Matlab keeps the
	   mexFunction stuff around across invocations, we must reinit the
	   static variables in addition to the Matlab variables. */
	_dods_err = 0;
	_intern_dods_err(_dods_err);
	_dods_err_msg[0] = '\0';
	_overflow_dods_err_msg = 0;
    }

	return(1);
}

/** Write a message to standard error. The matlab variables dods_err and
    dods_err_msg are not affected.

    @param m Write the message #m# to standard error. */
void 
msg(const char *m, ...) 
{
#ifdef WIN32
	char s[MSG_STR];
#endif

    va_list arg;
    va_start(arg, m);

#ifdef WIN32
	/*  There is no proper way output to the matlab window under   */
	/*  win32 except via mexPrintf().  We _must_ use that in the   */
	/*  win32 case.  I believe any *printf()'s aren't really valid */
	/*  for unix in cmex files, but they get by.  See the matlab   */
	/*  documentation for mexPrintf().  In any case, references to */
	/*  stdout, stdin, stderr are not valid for code executing     */
	/*  within win32 non-console programs even though they will    */
	/*  compile ok.  Perhaps we should go with this code in the    */
	/*  unix case also ??                                          */
	vsprintf(s, m, arg);
#else
    vfprintf(stderr, m, arg);
#endif

    va_end(arg);

#ifdef WIN32
	mexPrintf(s);
#endif
}

/** Write a message to the Matlab variable dods_err_msg. Also set the Matlab
    variable dods_err to 1. If err_msg() has been called since the last call
    to err_init(), this message is appended and the state of dods_err (which
    is already 1) is not changed.

    @param m Write the message #m# to the dods_err_msg Matlab variable. */
void 
err_msg(const char *m, ...) 
{
    va_list arg;
    va_start(arg, m);

    if (_comp_mode)
	{
#ifdef WIN32
	char s[MSG_STR];
	vsprintf(s, m, arg);
	mexPrintf(s);
#else
	vfprintf(stderr, m, arg);
#endif
	}
    else {
	char s[MSG_STR];
	vsprintf(s, m, arg);

	_dods_err = 1;
	_intern_dods_err(_dods_err);

	_append_new_msg(_dods_err_msg, s);
	_intern_msg(_dods_err_msg);
    }

    va_end(arg);

}

/** Write a message to the Matlab variable dods_err_msg including a system
    message. This works like perror, but dumps the result into the
    dods_err_msg variable. If perr_msg() or err_msg() has been called since
    the last call to err_init(), the message is appended and the state of
    dods_err (which is already 1) is not changed.

    The global errno is used to find the system error message. 

    @param m Write the message #m# to the dods_err_msg Matlab variable. */

void
perr_msg(const char *m, ...)
{
    va_list arg;
    va_start(arg, m);

    if (_comp_mode) {
	char *em = strerror(errno);
#ifdef WIN32
	char s[MSG_STR];
	vsprintf(s, m, arg);
	mexPrintf(s);
	mexPrintf("%s\n", em ? em: "");
#else
	vfprintf(stderr, m, arg);
	fprintf(stderr, "%s\n", em ? em: "");
#endif
    }
    else {
	char *em = strerror(errno);
	char s[MSG_STR];
	vsprintf(s, m, arg);
	sprintf(s, "%s\n", em ? em: "");

	_dods_err = 1;
	_intern_dods_err(_dods_err);

	_append_new_msg(_dods_err_msg, s);
	_intern_msg(_dods_err_msg);
    }

    va_end(arg);
}

#ifdef TEST

/* 
   Call with a.out <0|1> <message 0> ... <message N>. Each call to err_msg
   include three integer arguments.
*/

int
main(int argc, char *argv[])
{
    int i;

    err_init(atoi(argv[1]));

    i = 2;
    while (i < argc) {
	err_msg(argv[i], i, i, i);
	i++;
    };

    printf("dods_err: %d, dods_err_msg: `%s'\n", _dods_err, _dods_err_msg);
}

#endif

/* 
 * $Log: error.c,v $
 * Revision 1.1  2003/10/22 19:43:29  dan
 * Initial revision
 *
 * Revision 1.10  2001/08/27 18:06:57  jimg
 * Merged release-3-2-5.
 *
 * Revision 1.9.2.1  2000/12/13 01:33:31  jimg
 * Added defines.h or moved it after other preprocessor lines
 *
 * Revision 1.9  2000/11/22 23:43:00  jimg
 * Merge with pre-release 3.2
 *
 * Revision 1.3.2.9  2000/09/22 20:54:04  jimg
 * Added dmalloc stuff in DMALLOC guard
 *
 * Revision 1.8  2000/07/21 10:21:56  rmorris
 * Merged with win32-mlclient-branch.
 *
 * Revision 1.6.2.1  2000/06/26 23:02:21  rmorris
 * Modification for port to win32.
 *
 * Revision 1.7  2000/06/20 21:40:09  jimg
 * Merged with 3.1.6
 *
 * Revision 1.3.2.8  2000/06/20 20:36:46  jimg
 * Imporved error messages and fixed some doc++ comments (although some
 * problems remain).
 *
 * Revision 1.6  2000/04/20 23:38:13  jimg
 * Merged with release 3.1.3
 *
 * Revision 1.3.2.7  2000/04/20 19:54:48  jimg
 * *** empty log message ***
 *
 * Revision 1.3.2.6  2000/04/20 19:45:14  jimg
 * Changed some int variables to size_t.
 *
 * Revision 1.3.2.5  2000/04/19 05:58:33  jimg
 * Added two more calls to va_end().
 *
 * Revision 1.3.2.4  2000/04/14 16:46:02  edavis
 * Fixed some variable argument list stuff that died on DEC.
 *
 * Revision 1.5  2000/04/01 01:16:24  jimg
 * Merged with release-3-1-2
 *
 * Revision 1.3.2.3  2000/03/29 03:37:34  jimg
 * Added perr_msg() which prints a message along with a system error message
 * simplar to perror().
 *
 * Revision 1.3.2.2  2000/02/19 00:36:14  jimg
 * Switched from the ML 4 to 5 API.
 *
 * Revision 1.4  1999/11/09 06:43:41  jimg
 * result of merge with 3-1-1
 *
 * Revision 1.3.2.1  1999/11/02 23:49:37  jimg
 * Added instrumentation
 *
 * Revision 1.3  1999/07/24 00:09:29  jimg
 * Added include of config_writeval
 *
 * Revision 1.2  1999/04/30 17:06:57  jimg
 * Merged with no-gnu and release-2-24
 *
 * Revision 1.1.2.1  1999/04/29 20:10:26  jimg
 * Added
 * 
 */
