/* 
   (c) COPYRIGHT URI/MIT 1996,1997,1999,2000
   Please read the full copyright statement in the file COPYRIGHT.

   Authors:
      grf		Glenn Fleirl (glenn@lake.mit.edu)
      jhrg,jimg		James Gallagher (jgallagher@gso.uri.edu)
*/

/**
   loaddap is a Matlab 4 command which loads the variables specified by a
   DODS URL into Matlab. Because of limitations on the data types which
   Matlab 4 can process, there are some DODS datasets, or parts of some
   datasets, which cannot be read. Because the DODS Sequence data type is
   used by many important datasets, this software translates that type to an
   array or collection of arrays.

   jhrg 10/6/96 

   Notes: 
   1) Switched to Matlab 5 API. 2/17/2000 jhrg
   2) Don't check for null pointers from mxMalloc and mxCalloc; they report
   errors and exit the mexFunction when the heap is exhausted.
   3) No need to call mxFree since memory is freed automatically on exit from
   the mexFunction. However, don't overuse this since it can take Matlab a
   long time to free all the memory.

   @author Glenn Fleirl <glenn@lake.mit.edu>
   @author James Gallagher <jgallagher@gso.uri.edu>
*/

#include "config_writedap.h"

static char id[] not_used ={"$Id$"};

/* I think that all gnu x86 compilers define __i386__. 9/22/2000 jhrg */
#if defined(__i386__) && defined(__linux__) && defined(__GNUC__)
#define ARCH_GLNX86
#endif

#include <string.h>
#ifdef WIN32
#include <windows.h>
#include <conpipe.h>
#else
#include <unistd.h>
#endif

#ifdef ARCH_GLNX86
#include <dlfcn.h>		/* see lock_mex_function() */
#endif 

#include <mex.h>

#include "MLVars.h"
#include "extend.h"
#include "variable.h"

#ifdef WIN32
#define PATH_DELIMITER_STR ";"
#define PATH_DELIMITER_CHAR ';'
#define PATH_SEPARATOR "\\"
#define PATH_SEPARATOR_CHAR '\\'
#define isnan(x) _isnan(x)
#else
#include "queue.h"
#define PATH_DELIMITER_STR ":"
#define PATH_DELIMITER_CHAR ':'
#define PATH_SEPARATOR "/"
#define PATH_SEPARATOR_CHAR '/'
#endif

#ifdef WIN32
#define CONST
#else
#define CONST const
#endif

#if DMALLOC
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

#include "defines.h"

#define MATLAB_COMMAND "load %s"

#ifdef WIN32
#define DODS_BASE_COMMAND "writedap.exe"
#else
#define DODS_BASE_COMMAND "writedap"
#endif

#define BROWSER_COMMAND "netscape -remote 'openURL(%s)'"
#ifdef WIN32
#define DODS_COMPLETE_COMMAND "%s -f -n %s -- \"%s\""
#else
#define DODS_COMPLETE_COMMAND "%s -f -n %s -- %s"
#endif
#define DEFAULT_LOCATOR "http://unidata.ucar.edu/packages/dods/"
#define MAX_STR 256
#define BIG_STR 8192

#ifdef DODS_VERSION
static char *dods_version = DODS_VERSION;
#else
static char *dods_version = "unknown";
#endif /* VERSION */

int process_values(FILE *fin);

/** Does the user want verbose output messages. On by default. */
int verbose = 1;

/** Does the user want variables with the same names which are loaded two or
    more times in the same call to loaddap (you can pass more than one URL
    to loaddap) to be concatenated? */
int extend_existing_variables = 0;

/** This flag indicates that the user supplied the fullnames switch.
    Fullnames means that structure fields are interned by prefixing the
    field name with the name of the structure. This option is a hold over
    from the ML 4 days before structures (which still aren't fully supported
    8/17/2000 jhrg) and short (19 character) name limits (i.e., you probably
    did not want this on all the time. */
int fullnames = 0;

static char command_opts[MAX_STR] = ""; /* Options passed to writedap */

/** Does the user what the names of variables returned to result argument
    zero? */
int output_variable_names = 0;

/** If NUM_RETURN_ARGS is false, use the default mechanism where variables are
   directly interned in Matlab. If true, return values via plhs[]. If true
   the value of NUM_RETURN_ARGS is the number of values to return. After
   reading this many variables, discard any remaining ones. */
int num_return_args = 0;

/** Global array of arrays used to hold NUM_RETURN_ARGS. */
mxArray **return_args;

/** When using the return arguments feature of loaddap, this holds the
    current argument number. This is re-initialized by loaddap.cc:init()
    every time the loaddap command is run. 

    @see num_return_args
    @see return_args */
int current_arg = 0;

/** Return the DODS Root directory pathname. */

#if 0
static char *
dods_root()
{
    static char *dods_root = 0;

    if (!dods_root)
	dods_root = (getenv("DODS_ROOT") ? getenv("DODS_ROOT") : DODS_ROOT);

    return dods_root;
}
#endif

/** Escape all characters that the Bourne shell treats as special. Storage
  for the new string is allocated by this function and must be freed by the
  caller. */

static char *
quote_for_shell(char *src)
{
    char *c;
    size_t dest_size = 3 * strlen(src) + 1; /* Used in an a assert, too */
    char *dest = mxCalloc(dest_size, sizeof(char));
    DBGM(fprintf(stderr, "mxCalloc (%s:%d): %x\n", __FILE__, __LINE__, dest));
    
    *dest = '\0';
#ifdef WIN32
    //  No discrete thing called a 'shell' under win32, but
    //  some escaping is still required.  Most characters that have
    //  a meaning associated with them by the win32 command line
    //  are taken care of because we double quote the arg to writedap
    while ((c = strpbrk(src,"\""))) {
#else
    while ((c = strpbrk(src, " \"*?[];&()|^<>"))) {
#endif
	strncat(dest, src, c - src);
	strcat(dest, "\\");
	strncat(dest, c, 1);

	src = ++c;
    }
    strcat(dest, src);
    DBG(mexPrintf("quote_for_shell (%s:%d) src: %s\n", 
		  __FILE__, __LINE__, src));
    DBG(mexPrintf("quote_for_shell (%s:%d) dest: %s\n", 
		  __FILE__, __LINE__, dest));
    mxAssertS(strlen(dest) + strlen(src) + 1 <= dest_size, "");

    return dest;
}
			  
/** Get the pathname to writedap. Use the writedap executable that lives in
    the same place as loaddap. Ask Matlab which loaddap it's going to use
    and then assume that writedap will be in the same directory as loaddap.
    Note that while this function is intended to be used to find writedap, it
    really can be used to find any program so long as it lives in the same
    directory as this mex function.
*/
static char *
get_pathname(char *command)
{
    static char *writedap_path = NULL;
    mxArray *lhs[1];
    mxArray *rhs[1];
    char *pathname = NULL;
    char *path_sep = NULL;
    
    if (!writedap_path) {
	/* Get mex file name */
	rhs[0] = mxCreateString(mexFunctionName());

	/* Get full path to mex file */
	mexCallMATLAB(1, lhs, 1, rhs, "which");

	pathname = mxArrayToString(lhs[0]);
	path_sep = strrchr(pathname, PATH_SEPARATOR_CHAR);
	*path_sep = '\0'; /* end pathame at the last slash */
	writedap_path = mxMalloc(strlen(pathname) + strlen(command) + 2);
	mexMakeMemoryPersistent(writedap_path);

	strcpy(writedap_path, pathname);
	strcat(writedap_path, PATH_SEPARATOR);
	strcat(writedap_path, command);

#ifndef DMALLOC
	/* Don't do this if usig dmalloc since pathname was allocated by
	   Matlab, not dmalloc and dmalloc won't let you free it. 9/22/2000
	   jhrg */
	mxFree(pathname);
#endif
    }

    return writedap_path;
}

/** Lock the mex function in memory. This is a work-around for a bug in gcc
    on linux. See Mathworks solution #26304. This is not the same as the
    Matlab function mexLock(). */
static void
lock_mex_function()
{
#ifdef ARCH_GLNX86
  DBG(msg("In lock_mex_function\n"));
  if (!mexIsLocked()) {
    mxArray *lhs[1];
    mxArray *rhs[1];
    char *pathname = NULL;
    
    DBG(msg("In locking mex function!\n"));
    mexLock();
    
    /* Get mex file name */
    rhs[0] = mxCreateString(mexFunctionName());

    /* Get full path to mex file */
    mexCallMATLAB(1, lhs, 1, rhs, "which");

    /* Permanantly load mex file */
    pathname = mxArrayToString(lhs[0]);
    if (dlopen(pathname, RTLD_NOW) == NULL)
      mexErrMsgTxt("Could not load mex file!");

    /* Free allocated memory */
#ifndef DMALLOC
    /* Don't do this if usig dmalloc since pathname was allocated by Matlab,
       not dmalloc and dmalloc won't let you free it. 9/22/2000 jhrg */
    mxFree(pathname);
#endif
  }
#endif
}

/** Read the command arguments and perform initial actions. If the command is
  called with no argument or with a "*" or "?", start up the WWW browser and
  open the default DODS locator page. If the argument is "*" or "?" followed
  by a URL then use that URL as the `locator page'. Finally, if the command
  is given a URL (without a prefix of "*" or "?") return that without any
  further actions. If the command argument ends with the suffix ".mat",
  assume that it is a Matlab `mat' file and load it without any additional
  processing.
  
  NB: Memory allocated by mxCalloc() is automatically freed on exit from the
  command (by Matlab), so there is no need to call mxFree() on mxCalloc'd
  blocks. 

  @return A URL to a DODS dataset or NULL if no further action is required. */

static char *
init(int nlhs, mxArray *plhs[], const int nrhs, CONST mxArray *prhs[])
{
    int i;			/* Count prhs[] elements. */
    char *s = mxCalloc(MAX_QUEUE_LEN, sizeof(char)); /* Return this array. */
    DBGM(mexPrintf("mxCalloc (%s:%d): %x\n", __FILE__, __LINE__, s));

    for (i = 0; i < nrhs; ++i) {
	if (!mxIsChar(prhs[i])) {
	    msg("usage: Arguments must be strings; type `help loaddap'\n");
	    return NULL;
	}
    }

    /* If there are two arguments, assume that the first is a set of command
       line options. Extract them and set the global `command_opt' string. If
       there is only one string, assume it is a URL and that no options were
       given, unless either the first character is a `-' or the only
       character is `*'. For some obscure reason, ? also works as *. */

    if (nrhs == 0) {
	command_opts[0] = '\0';
	s[0] = '\0';
    }
    else if (nrhs == 1) {
	command_opts[0] = '\0';
	mxGetString(prhs[0], s, (MAX_QUEUE_LEN-1));
	/* If the first char of `s' is `-' assume that s contains options 
           and since nrhs == 1, that no URL or filename was given. */
	if (s[0] == '-') {
	    strncpy(command_opts, s, (MAX_STR-1));
	    command_opts[(MAX_STR-1)] = '\0';
	    s[0] = '\0';
	}
    }
    else if (nrhs == 2) {
	mxGetString(prhs[0], command_opts, (MAX_STR-1));
	mxGetString(prhs[1], s, (MAX_QUEUE_LEN-1));
    }	
    else {			/* (nrhs > 2 || nrhs < 1) */
	msg("usage: loaddap [options] [url | `*' | `?']\n");
	return NULL;
    }

    /* Scan the command options string and set various flags. */

    /* Check for the `-e' option and call err_init(). */
    {
    	char *err_opt;
	if ((err_opt = strstr(command_opts, "-e"))) {
	    err_init(0);	/* user asks for new scheme */
	    /* blank out `-e' since writedap does not understand it */
	    (*err_opt) = ' '; *(err_opt + 1) = ' '; 
	}
	else 
	    err_init(1);	/* else, use compatibility mode */
    }

    /* Print the version of both the loaddap and writedap programs. */
    if (strstr(command_opts, "-V")) {
	FILE *fin;
	char *pathname;
	char *command;

	pathname = get_pathname(DODS_BASE_COMMAND);
	if (!pathname) {
	    err_msg(\
"Error: Could not find the loaddap helper application (writedap).\n\
       This error may be caused by several things: writedap might be\n\
       missing, or might not be in a directory where loaddap looks\n\
       (loaddap always looks in its own directory, plus PATH and\n\
       MATLABPATH).\n");
	    return NULL;
	}

	command = mxCalloc(strlen(command_opts) + strlen(pathname) 
			   + strlen("2>&1") + strlen(DODS_COMPLETE_COMMAND) 
			   + 1, sizeof(char));
	DBGM(fprintf(stderr, "mxCalloc (%s:%d): %x\n", __FILE__, __LINE__, 
		     command));

	/* If the user called loaddap('-V') using the assignment syntax,
	   then make user that the version number string is assigned to the
	   LHS identifier given by the user. Otherwise, dump the version
	   strings to stderr. 4/20/99 jhrg */
	if (nlhs > 0) {
	    int error = 0;
	    char ver[MAX_STR];
            /* Route stderr (which is where writedap sends the version
               information) to stdout so that we can read it from pin. */
#ifdef WIN32
           sprintf(command, DODS_COMPLETE_COMMAND, pathname, 
              command_opts, "");
           DBG(msg("command: %s\n", command));
           fin = openpipe(command, true);
#else
           sprintf(command, DODS_COMPLETE_COMMAND, pathname, 
              command_opts, "2>&1");
           DBG(msg("command: %s\n", command));
           fin = popen(command, "r");
#endif
	    fgets(ver, MAX_STR, fin);
	    DBG(msg("Writedap version string: %s\n", ver));
	    pclose(fin);
	    strcat(ver, "loaddap: ");
	    strcat(ver, dods_version);
	    plhs[0] = mxCreateString(ver);
	    mxSetName(plhs[0], "dods_version");
	}
	else {
	    sprintf(command, DODS_COMPLETE_COMMAND, pathname, 
		    command_opts, "");
	    DBG(msg("command: %s\n", command));
#ifdef WIN32
	    fin = openpipe(command, false);
#else
	    fin = popen(command, "r");
#endif
	    pclose(fin);
	    msg("loaddap: %s\n", dods_version);
	}

	DBGM(mexPrintf("mxFree (%s:%d): %x\n", __FILE__, __LINE__, 
		     pathname));
	/* mxFree(pathname); */
	return NULL;
    }

    /* Check for `+v' anti-verbose option. If not present, use verbose mode
       of writedap (since -v is the default for loaddap). */
    {
	char *no_verb;
	if ((no_verb = strstr(command_opts, "+v"))) {
	    verbose = 0;
	    /* blank out `+v' since writedap does not understand it */
	    (*no_verb) = ' '; *(no_verb + 1) = ' '; 
	}
	else if (!strstr(command_opts, "-v"))
	    strcat(command_opts, " -v");
    }

    /* Check for `-k' catenate option. If present, turn on variable
       catenation by setting the global extend_existing_variables. Off by 
       default.

       Note that writedap is clueless about this option. */
    {
	char *kat;
	if ((kat = strstr(command_opts, "-k"))) {
	    extend_existing_variables = 1;
	    /* blank out `-k' since writedap does not understand it */
	    (*kat) = ' '; *(kat + 1) = ' '; 
	}
	else
	    extend_existing_variables = 0;
    }

    /* Check for the `fullnames' option. If present, name each variable using
       all the names of the constructor types that contain the variable.
       Otherwise, name variables using just the leaf variable's name. NB:
       Since Matlab 4 limits variable names to 31 characters it is easy to
       create names that are too long using this option. It is better to use
       writedap's -r option to rename variables. */
    {
	char *full;
	if ((full = strstr(command_opts, "-n"))) {
	    fullnames = 1;
	    /* blank out `-n' since writedap does not understand it */
	    (*full) = ' '; *(full + 1) = ' '; 
	}
	else
	  fullnames = 0;
    }

    /* Did the user supply `-s'? If so, output a vector/matrix containing the
       names of each of the variables returned via the `return args' feature.
       Note that this has no effect when interning variables directory into
       the Matlab workspace. */
    {
	char *names;
	if ((names = strstr(command_opts, "-s"))) {
	    if (nlhs > 0) {	/* only return values if they are there! */
		output_variable_names = 1;
	    }
	    else {
		err_msg(\
"Error: The -s option (return a vector of variable names) is is only \n\
       available when using loaddap with return arguments. See `help\n\
       loaddap' for more information.\n");
	    }
	    /* blank out `-s' since writedap does not understand it */
	    (*names) = ' '; *(names + 1) = ' '; 
	}
	else
	    output_variable_names = 0;
    }

    /* END of option processing code */

    /* Now deal with the return arguments. If nlhs > 0 use plhs[] for the
       return values, otherwise directly intern values. */
    if (nlhs > 0) {
	num_return_args = nlhs;
	return_args = &plhs[0];
	if (output_variable_names)
	    current_arg = 1;	/* First return arg is string matrix. */
	else
	    current_arg = 0;
	if (extend_existing_variables) {
	    err_msg(\
"Error: The option `-k' (extend existing variables) has no effect when \n\
       using loaddap with a vector of return variables\n");
	    extend_existing_variables = 0;
	}
    }
    else {
	num_return_args = 0;
    }

#ifndef WIN32  /*  We're omitting the communication w/browser under win32  */
    if (s[0] == '*' || s[0] == '?' || s[0] == '\0') {
	key_t msgqid = 3075;
	long queue_id;

	if (strlen(s) == 1)
	    strcpy(s, DEFAULT_LOCATOR);

	if (strlen(s) > 1) {
	    char *command = mxCalloc(strlen(s) + strlen(BROWSER_COMMAND) + 1,
				     sizeof(char));
	    DBGM(fprintf(stderr, "mxCalloc (%s:%d): %x\n", __FILE__,
			 __LINE__, command));

	    sprintf(command, BROWSER_COMMAND, s);
	    DBG(msg("Command: %s\n", command));
	    system(command);
	}

	/* Read the URL from the WWW browser (using a SYS V queue). */

	if (questart(&msgqid) < 0)
	    err_msg("Internal Error: Queue exists already. (%s:%d)\n", 
		    __FILE__, __LINE__);

	queue_id = queconn(&msgqid);
	msg("Waiting for URL from browser...\n");

	if (queread(queue_id, s, 10) < 0) {
	    err_msg("Internal Error: Cannot read from queue. (%s:%d)\n", 
		    __FILE__, __LINE__);
	    return NULL;
	}
	
	DBG(msg("Read `%s' from the browser\n", s));
	questop(queue_id);
    }
#endif

    return s;
}

void 
mexFunction(int nlhs, mxArray *plhs[], int nrhs, CONST mxArray *prhs[])
{
    char *command = 0;
    char *dods_url = 0, *dods_url_escaped = 0;
    char *pathname = 0;
    FILE *fin;

#ifdef ARCH_GLNX86
    lock_mex_function();
#endif

    /* Get the URL and set global options. */
    if (!(dods_url = init(nlhs, plhs, nrhs, prhs)))
	goto exit;

    /* Build the writedap command, including options and the URL. Memory
       freed at exit. */
    dods_url_escaped = quote_for_shell(dods_url);

    /* get_pathname returns a pointer to a static array; don't free it. */
    pathname = get_pathname(DODS_BASE_COMMAND);
    if (!pathname) {
	err_msg(\
"Error: Could not find the loaddap helper application (writedap).\n\
       This error may be caused by several things: writedap might be\n\
       missing, or might not be in a directory where loaddap looks\n\
       (loaddap always looks in its own directory, plus PATH and\n\
       MATLABPATH).\n");
	goto exit;
    }

    /* Memory for command freed at exit. */
    command = mxCalloc(strlen(pathname) +strlen(dods_url_escaped) 
		       + strlen(command_opts) + strlen(DODS_COMPLETE_COMMAND)
		       + 1, sizeof(char));
    DBGM(mexPrintf("mxCalloc (%s:%d): %x\n", __FILE__, __LINE__, 
		 command));

    sprintf(command, DODS_COMPLETE_COMMAND, pathname, command_opts, 
	    dods_url_escaped);
    DBG(msg("command: %s\n", command));

    /* Run the command. */
#ifdef WIN32
    fin = openpipe(command, false);
#else
    fin = popen(command, "r");
#endif
    if (!fin || feof(fin) || ferror(fin)) {
	err_msg(\
"Internal Error: loaddap found its helper application (writedap), but\n\
                could not get it to run. Try running writedap yourself\n\
                to see if there's an obvious solution to the problem\n\
                (type `./writedap -h' for help on running writedap). \n\
                Otherwise, contact support@unidata.ucar.edu.\n");
	goto exit;
    }

    if (!process_values(fin))
      goto exit;
    
    if (ferror(fin)) {
	err_msg(\
"Error: The loaddap helper application writedap reported an error. This\n\
       may be due to an earlier error. However, if there was no previous\n\
       error message, please report this to support@unidata.ucar.edu.\n");
	goto exit;
    }

#ifndef WIN32
    /*  If we got to this point via an error while procesing the output of
     *  writedap, we stopped consuming the input.  As a result, writedap
     *  may be sitting stuck in memory waiting for its stdout to be
     *  consumed before exiting.  We have to allow writedap to exit.
     */
    {
	char c;
	while (!feof(fin))
	    fread(&c, 1, 1, fin);
    }

    {
	int status;
	status = pclose(fin);
#if 0
	if (status != 0)
	    err_msg(\
"Error: The loaddap helper application writedap reported an error (%d). \n\
       This may be due to an earlier error. However, if there was no previous\n\
       error message, please report this to support@unidata.ucar.edu.\n",
                    status);
#endif
    }

    fflush(stderr);
    fflush(stdout);
#endif

 exit:
    /* Note: I'm freeing all this memory because the Matlab API guide
       suggests that. The mxMalloc, ..., functions all automatically free
       memory (if it is not marked persistent) but that is apparently not
       very efficient. 2/18/2000 jhrg 

       Also note that the pointer variables are set to null once I've freed
       their memory. This ensures that those variables will have the same
       (null) values every time loaddap is started. 2/22/2000 jhrg */

    /* Clean up the list of variable names. This is run here because mxMalloc
       is used to allocate memory for the variable name list (see extend.c)
       and that memory is automatically freed when the mexFunction exists.
       Thus, if we call this at the beginning of the mexFunction, on the
       second, ..., invocation we'll be freeing memory that has already been
       freed. Not a good thing. 2/18/2000 jhrg */
    clear_existing_variables();

    /* dods_url now points to a static string created in init(...). */
    if (dods_url) {
	DBGM(mexPrintf("mxFree (%s:%d): %x\n", __FILE__, __LINE__, 
		     dods_url));
	mxFree(dods_url);
	dods_url = 0;
    }

    if (dods_url_escaped) {
	DBGM(mexPrintf("mxFree (%s:%d): %x\n", __FILE__, __LINE__, 
		     dods_url_escaped));
	mxFree(dods_url_escaped);
	dods_url_escaped = 0;
    }

    if (command) {
	DBGM(mexPrintf("mxFree (%s:%d): %x\n", __FILE__, __LINE__, 
		     command));
	mxFree(command);
	command = 0;
    }

#if DMALLOC
    dmalloc_shutdown();
#endif

    return;
}

/* 
 * $Log: loaddap.c,v $
 * Revision 1.3  2004/07/08 20:50:03  jimg
 * Merged release-3-4-5FCS
 *
 * Revision 1.1.2.2  2004/04/30 14:59:51  dan
 * Added space ' ' character to set for escaping in quote_for_shell().
 *
 * Revision 1.2  2003/12/08 17:59:50  edavis
 * Merge release-3-4 into trunk
 *
 * Revision 1.1.2.1  2003/10/27 16:59:42  dan
 * Modified versions of 'loaddods' and 'writeval' using
 * new naming conventions for 'loaddap' and 'writedap', respectively.
 *
 * Revision 1.1.1.1  2003/10/22 19:43:31  dan
 * Version of the Matlab CommandLine client which uses Matlab Structure
 * variables to maintain the shape of the underlying DODS data.
 *
 * Revision 1.52  2003/01/29 15:43:52  dan
 * Resolved conflict on merge, caused by PWEST's removal
 * of the SLLIST includes in a previous update on the trunk.
 *
 * Revision 1.51  2001/10/14 01:12:27  jimg
 * Merged with release-3-2-7.
 * CVg
 *
 * Revision 1.49.2.9  2002/05/19 06:33:59  rmorris
 * Fixes to handle some of the more exotic characters in a URL for win32.
 * No shell to escape for but the win32 command line has similiar ideas for
 * special characters.
 *
 * Revision 1.49.2.8  2001/11/06 13:19:19  rmorris
 * Modifications while debugging for win32 runtime.  mxDestroyArray
 * should not be called on "lhs".  See the matlab documentation for
 * mxDestroyArray for specifics - "You should not call mxDestroyArray
 * on an mxArray you are returning on the left-hand side".  Memory
 * was getting trashed on win32 as a result.  Also, there was an attempt
 * to free static memory allocated for the "pathname" using mxFree and
 * this was also causing out-of-bounds memory access under win32.
 *
 * Revision 1.49.2.7  2001/10/10 21:14:37  jimg
 * Changed include of mex.h from "mex.h" to <mex.h> so that it won't be
 * included in the Makefile.in's dependencies. This maens builders are
 * less likely to have to run make depend.
 *
 * Revision 1.50  2001/08/27 18:06:57  jimg
 * Merged release-3-2-5.
 *
 * Revision 1.49.2.6  2001/05/15 18:29:44  jimg
 * Added some program instrumentation.
 *
 * Revision 1.49.2.5  2000/12/15 18:11:07  edavis
 * Change some C++ style comments to C style comments.
 * Not a problem with gcc but when building on SGI I use cc
 * to build the o32 version and it chokes.
 *
 * Revision 1.49.2.4  2000/12/14 05:59:15  jimg
 * Moved code that closes the pipe to writeval before the exit symbol. This
 * fixes a bug that shows up when users request the version string. The pipe was
 * being closed twice.
 *
 * Revision 1.49.2.3  2000/12/13 01:26:00  jimg
 * Removed an extra set of calls to read extra chars from writeval and call
 * pclose().
 *
 * Revision 1.49.2.2  2000/12/07 23:25:38  jimg
 * Replaced get_pathname() with a version that uses Matlab and its `which'
 * command to locate writeval. This is much simpler than the older
 * get_pathname() which worked OK unless writeval was in a directory on PATH
 * and/or was actually a softlink. In that case loaddods would seg fault. The
 * new version limits writeval to the same directory that holds loaddods, but
 * seems much more robust.
 *
 * Revision 1.49.2.1  2000/12/07 22:11:11  jimg
 * Fixed the call to get_pathname() when -V is given. The code was hardwired to
 * use only a local copy of writeval. Fixed. I also added a new `find writeval'
 * function, but it's not being used now since the get_pathname stuff works.
 *
 * Revision 1.49  2000/11/22 23:43:00  jimg
 * Merge with pre-release 3.2
 *
 * Revision 1.40.2.19  2000/09/22 21:00:32  jimg
 * Added code to lock the mex function when run under linux. This Solution to
 * the problem of Matlab crashing at exit after running loaddods is from
 * Mathworks Solution #26304. Even though that solution addresses C++, it
 * seems to apply here as well. Also modified get_pathname() to copy
 * environment variable value only once.
 *
 * Revision 1.48  2000/08/30 00:13:29  jimg
 * Merged with 3.1.7
 *
 * Revision 1.40.2.18  2000/08/25 23:35:38  jimg
 * Modified to use the new writeval -n option (now loaddods calls writeval
 * using `-n -f' in addition to whatever other options the user chooses.
 *
 * Revision 1.40.2.17  2000/08/17 23:56:48  jimg
 * Factored the code that reads and processes variables and values into
 * process_values.c (from loaddods.c).
 *
 * Revision 1.47  2000/07/21 10:21:56  rmorris
 * Merged with win32-mlclient-branch.
 *
 * Revision 1.45.2.6  2000/07/18 09:56:36  rmorris
 * Moved win32 pipe from console app functionality out of this file and
 * into (win32-specific) source files conpipe.c and conpipe.h.   General
 * cleanup to further attempt to minimize the number of ifdef's.
 *
 * Revision 1.45.2.5  2000/07/16 09:34:33  rmorris
 * Removed win95/98 specific code, replacing with generic win32 solution
 * for spawing console application writeval.
 *
 * Revision 1.45.2.4  2000/07/14 21:36:04  rmorris
 * Modifications to make sure writeval gets terminated under win32 and
 * to make a runtime decision (based on win32 OS being used) to spawn
 * (or not) a 3rd process that handles pipe (to writeval) closure
 * appropriately in the win9x case.  The win9x-specific code hasn't been
 * tested as of 7/2000.
 * case.
 *
 * Revision 1.45.2.3  2000/07/13 07:11:02  rmorris
 * Accomodate differences in the behavior of pipes in win32 (vs. unix).
 *
 * Revision 1.45.2.1  2000/06/26 23:02:21  rmorris
 * Modification for port to win32.
 *
 * Revision 1.46  2000/06/20 21:40:09  jimg
 * Merged with 3.1.6
 *
 * Revision 1.40.2.16  2000/06/20 20:34:07  jimg
 * Fixed bugs in do_attributes. Both the values of use_structures and verbose
 * were not handled properly. Verbose behavior should be turned off for the
 * attribute code since some of the do_*() functions print messages about adding
 * variables to the user's workspace. Also, the use_structures switch was left
 * on after a call with -A. This is a problem when reading regular data.
 * Also, various error messages were improved and some messages that print out
 * confusing messages were removed.
 *
 * Revision 1.45  2000/06/15 00:01:32  jimg
 * Merged with 3.1.5
 *
 * Revision 1.40.2.15  2000/06/14 20:45:14  jimg
 * Added read_string_value function.
 * Fixed do_string so that it handles multi-line strings if they are quoted
 * with double quotes.
 *
 * Revision 1.40.2.14  2000/06/09 22:38:40  edavis
 * Fix some minor (?) problems discovered during SGI build.
 *
 * Revision 1.44  2000/06/07 00:28:32  jimg
 * Merged changes from version 3.1.4
 *
 * Revision 1.40.2.13  2000/06/02 23:03:33  jimg
 * Fixes for AttrTables with no entries.
 * Added support for name translation and a DODS_ML_Real_Name attribute.
 * Changed the `size' attribute's name to DODS_ML_Size.
 *
 * Revision 1.40.2.12  2000/06/01 01:33:53  jimg
 * Added support for writeval's new attribute mode.
 *
 * Revision 1.40.2.11  2000/05/04 03:19:13  jimg
 * Changed probe_path so that paths (e.g., PATH) are not over run. This can
 * happen when the program (writeval) is not found and the path does not end in a
 * colon.
 *
 * Revision 1.40.2.10  2000/04/25 22:48:10  edavis
 * Undo some int to size_t changes that caused problems on DEC build.
 * Cast to size_t in locations where necessary.
 *
 * Revision 1.40.2.9  2000/04/20 19:45:15  jimg
 * Changed some int variables to size_t.
 *
 * Revision 1.40.2.8  2000/04/19 05:39:53  jimg
 * Added conditional efence malloc defines.
 *
 * Revision 1.40.2.7  2000/04/15 02:20:53  jimg
 * Removed strtok/strtok_r. strtok_r caused problems on Linux and strtok is
 * a poor function.
 *
 * Revision 1.40.2.6  2000/04/13 18:30:19  jimg
 * Changed the strtok_r calls to strtok. The strtok_r calls made Matlab on Linux
 * (Redhat 5) gag.
 *
 * Revision 1.40.2.5  2000/04/11 21:47:39  jimg
 * Removed tests that prevented interning variables with more than two
 * dimensions. Note that n-dim support is now in extend.c but the -k option is
 * not supported for more than 2 dims.
 *
 * Revision 1.40.2.4  2000/04/01 00:40:23  jimg
 * Increased the size of BIG_STR so that long attributes will work
 *
 * Revision 1.40.2.3  2000/04/01 00:10:04  jimg
 * Changed the way dimensions are handled so that n-dimension versions of the
 * extend functions can be supported.
 * Fixed dynamic memory errors - there seems to be a small leak, but loaddods
 * can still run for hours.
 *
 * Revision 1.40.2.2  2000/02/19 00:36:14  jimg
 * Switched from the ML 4 to 5 API.
 *
 * Revision 1.40.2.1  1999/11/02 23:49:37  jimg
 * Added instrumentation
 *
 * Revision 1.40  1999/07/24 00:10:29  jimg
 * Merged the release-3-0-2 branch
 *
 * Revision 1.39  1999/04/30 17:06:57  jimg
 * Merged with no-gnu and release-2-24
 *
 * Revision 1.38.2.2  1999/04/29 20:13:00  jimg
 * Removed old code.
 *
 * Revision 1.38.2.1  1999/04/29 20:07:50  jimg
 * Fixed error messages so that they either print text to stderr *or* store
 * text in a matlab variable called dods_err_msg. Use the -w flag to get this
 * behavior.
 * Changed the -V option so that it returns the version info to a matrix when
 * loaddods is called using the assignment syntax.
 *
 * Revision 1.38  1999/03/29 21:25:36  jimg
 * Added support for the new datatypes
 *
 * Revision 1.37  1999/02/08 20:55:00  jimg
 * Fixed a bug where PATH values longer than 256 chars were not searched. This
 * meant that if users put writeval in a directory that was listed at the end of
 * their path and strlen(PATH) was > 256 chars, writeval would not be found.
 * However, it *would* be found if listed at the front of PATH. I made the
 * software more general and, at the same time, added search of MATLABPATH. That
 * is, now DODS_ROOT/{etc,bin}, PATH, MATLABPATH and the CWD are all searched
 * when loaddods looks for writeval.
 *
 * Revision 1.36  1998/11/30 05:37:36  jimg
 * Added the -T option to control truncation of names longer than 19 characters
 *
 * Revision 1.35  1998/11/25 01:15:23  jimg
 * Identifiers limited to 19 chars. This is a temp fix; will update to include ML 5 later
 *
 * Revision 1.34  1998/11/20 08:41:22  jimg
 * Fixed the fix for the extra newline separators; it was broken for arrays
 *
 * Revision 1.33  1998/11/19 20:54:37  jimg
 * Repaired some error messages
 *
 * Revision 1.32  1998/11/10 03:18:14  jimg
 * Added test stuff. This is fairly hokey...
 *
 * Revision 1.31  1998/06/09 04:04:36  jimg
 * Fixed get_pathname.
 * Added exit(1) to do_error().
 *
 * Revision 1.30  1998/03/21 00:20:54  jimg
 * Fixed string internment so that it now works with vectors of strings (using
 * reorganize_strings()).
 *
 * Revision 1.29  1998/03/19 23:24:36  jimg
 * Added do_error(). It reads error messages relayed by writeval and displays
 * them using mexErrMsgTxt().
 *
 * Revision 1.28  1997/12/16 02:58:06  jimg
 * Merged release 2.14d changes
 *
 * Revision 1.27  1997/10/24 23:52:43  jimg
 * Added to get_pathname() so that the CWD is searched first.
 *
 * Revision 1.26  1997/10/09 22:20:05  jimg
 * Resolved conflicts in merge of 2.14c to trunk.
 *
 * Revision 1.25  1997/10/04 00:33:48  jimg
 * Release 2.14c fixes
 *
 * Revision 1.23.6.1  1997/09/19 19:11:22  jimg
 * Moved config_writedap.h so that -V4_COMPAT flag works.
 *
 * Revision 1.24  1997/09/10 22:49:56  jimg
 * Fixed a bug that shows up on Solaris 2 where the pipe connecting this
 * program to writeval must be flushed before calling pclose().
 *
 * Revision 1.23  1997/05/06 00:34:26  jimg
 * Updated version
 *
 * Revision 1.22  1997/05/01 22:40:59  jimg
 * loaddods can now return arguments using Matlab's assignment operator:
 * [a b c] = loaddods('...'); In addition, it now accepts the option -s which
 * causes the first return argument to be a vector of strings (elements
 * separated by a tab) naming the variables in the following return arguments.
 *
 * Revision 1.21  1997/04/29 23:42:39  jimg
 * Fixed a bug where loaddods crashed Matlab when invoked with '-v' and no
 * other options. This also fixes the bug where command line options broke
 * reading from the urlqueue program.
 *
 * Revision 1.20  1997/04/21 20:29:23  jimg
 * Updated version to 1.4 - this matches version 2.13a of DODS.
 *
 * Revision 1.19  1997/04/21 18:29:10  jimg
 * get_pathname() now performs tilde (~) expansion ala csh(1).
 *
 * Revision 1.18  1997/04/19 00:49:01  jimg
 * Now checks the user's PATH and DODS_ROOT/etc before calling popen(). popen()
 * does not return an error if the shell (sh) runs but cannot find the program.
 *
 * Revision 1.17  1997/04/18 22:45:53  jimg
 * Added #include <sys/msg.h> for linux.
 * Removed sys/wait.h since I now use popen/pclose.
 *
 * Revision 1.16  1997/04/18 21:12:26  jimg
 * Fixed a bug which (initially) showed up only on Solaris (but later on
 * SunOS too) where Grids whose size was a power of 2 failed to load. The
 * problem was caused by using fseek() to read a newline character that
 * followed the data. I the course of finding this bug I removed all use
 * of fscanf() and isolate reads from the writeval subprocess into a
 * handful of read_*() functions. Also added was a `test mode' where
 * loaddods could be built as a regular UNIX executable (to simplify
 * debugging).
 *
 * Revision 1.15  1997/03/27 22:57:59  jimg
 * Added code on do_array to ensure that only column-major data is interned in
 * Matlab. This currently entails a stop-and-copy type operation. A more
 * thorough fix would transform the row-major data from writeval while reading
 * from the pipe...
 *
 * Revision 1.14  1997/03/27 18:34:02  jimg
 * Fixed quoting bug - double quotes were not escaped with backslash.
 *
 * Revision 1.13  1997/03/23 22:33:15  jimg
 * Changed so that concatenation is off by default. Use `-k' to activate it.
 * Added `-n' option for the fullnames feature.
 *
 * Revision 1.12 1997/03/07 04:47:04 jimg Added catenation option. Variables
 * are by default catenated when they have the same name. Note that this only
 * works for discrete calls to loaddods. That is, if you call loaddods(...)
 * and it contains several URLs which read variables with the same name,
 * those vectors or arrays will be catenated into single variables. Use the
 * `+k' option to suppress this.
 *
 * Revision 1.11  1997/02/11 16:44:26  jimg
 * Fixed a bug in quote_for_shell() where trailing non-quoted chars were
 * truncated.
 *
 * Revision 1.10  1997/02/06 19:38:42  jimg
 * Added constants for string sizes.
 * Fixed the `absolute path' bug where writeval would only be run if it was in
 * the user's CWD.
 * Added recognition of NaN; when writeval emits NaN for a Float64 value,
 * loaddods uses Matlab's mexGetNaN to intern the value.
 * Verbose mode is now more informative.
 *
 * Revision 1.9  1997/02/03 20:19:53  jimg
 * Removed startchild() from loaddods and used popen instead. The code that
 * checks for the processes started by startchild() does not work correctly,
 * resulting in a `no more pipes' error message when loaddods is used
 * repeatedly. popen()/pclose() fix that problem.
 * Also, removed use of exit() since it causes the matlab process itself to
 * quit.
 *
 * Revision 1.8  1997/01/08 21:32:47  jimg
 * Added UInt32 to set of scalar types processed by do_array(). This fixes a
 * problem where arrays of UInt32 were not processed properly.
 * Added UInt32 to the set of type names that loaddods will process using
 * do_float().
 *
 * Revision 1.7  1996/12/21 00:42:54  jimg
 * Fixed a nasty bug where single dimension arrays resulted in HUGE chunks of
 * memory being allocated. See do_array() and its use of mexPutFull().
 *
 * Revision 1.6  1996/12/18 00:50:34  jimg
 * Added the ability to process command switches which are passed to writeval.
 * loaddods can be invoked `loaddods('-g', '*')' to get the DODS list in
 * netscape and have the GUI displayed by writeval.
 *
 * Revision 1.5  1996/10/29 21:40:34  jimg
 * Added fullnames global that controls whether interned variables are named
 * based on their position in the DDS hierarchy (fullnames == true) or based on
 * their `leaf name' only (fillnames == false). The default is false since when
 * true names can become too long for Matlab.
 * Added error checking for some more of the mexPutFull calls.
 * Replaced fscanf reads of newline with fseek() calls since on the Dec Alpha
 * the hex sequence 0a0c (newline followed by 12(dec)) is read for "\n" by
 * fscanf() when this software expects that only the 0x0a byte will be read.
 * This bug may not be limited to the Dec Alpha, however, since that machine is
 * little endian the 0x0a0c pattern showed up frequently.
 *
 * Revision 1.4  1996/10/25 22:59:52  jimg
 * Added default locator as constant.
 * Added debugging statements.
 * Fixed problems with do_grid.
 * Added verbose flag - there is currently no way to control this option, it is
 * always on.
 * Removed do_function - use do_sequence instead.
 *
 * Revision 1.3  1996/10/23 23:58:39  jimg
 * Massive rewrite so that recursive data types are not flattened by instead
 * are printed `recursively'.
 *
 * Revision 1.2  1996/10/07 20:56:11  jimg
 * Broke out the queue functions to their own file (which is also used by
 * urlqueue).
 * Added header. */
