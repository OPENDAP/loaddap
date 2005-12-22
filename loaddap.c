/*
// -*- mode: c++; c-basic-offset:4 -*-

// This file is part of loaddap.


// Copyright (c) 2005 OPeNDAP, Inc.
// Author: James Gallagher <jgallagher@opendap.org>
//
// This is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
// 
// This software is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
// You can contact OPeNDAP, Inc. at PO Box 112, Saunderstown, RI. 02874-0112.
*/

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
#include <errno.h>
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

#if 0
#include "queue.h"
#endif

#ifdef WIN32
#define PATH_DELIMITER_STR ";"
#define PATH_DELIMITER_CHAR ';'
#define PATH_SEPARATOR "\\"
#define PATH_SEPARATOR_CHAR '\\'
#define isnan(x) _isnan(x)
#else
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
#define DODS_COMPLETE_COMMAND "%s -f -n %s -- \"%s\""
#else
#define DODS_BASE_COMMAND "writedap"
#define DODS_COMPLETE_COMMAND "%s -f -n %s -- %s"
#endif

#if 0
#define BROWSER_COMMAND "netscape -remote 'openURL(%s)'"
#define DEFAULT_LOCATOR "http://unidata.ucar.edu/packages/dods/"
#endif

#define MAX_STR 256
#define BIG_STR 8192
#define MAX_URL_LEN 8192

static char *dods_version = PACKAGE_VERSION;

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
	/* Don't do this if using dmalloc since pathname was allocated by
	   Matlab, not dmalloc and dmalloc won't let you free it. 9/22/2000
	   jhrg */
	mxFree(pathname);
#endif
    }

    return writedap_path;
}

/** Lock the mex function in memory. This is a work-around for a bug in gcc
    on linux. See Mathworks solution #26304. This is not the same as the
    Matlab function mexLock(). 
    
    This may no longer be necessary. 10/4/05 jhrg */
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
  
  @note Memory allocated by mxCalloc() is automatically freed on exit from the
  command (by Matlab), so there is no need to call mxFree() on mxCalloc'd
  blocks. 

  @note Support for the browser option has been removed.

  @return A URL to a DODS dataset or NULL if no further action is required. */

static char *
init(int nlhs, mxArray *plhs[], const int nrhs, CONST mxArray *prhs[])
{
    int i;			/* Count prhs[] elements. */
    char *s = mxCalloc(MAX_URL_LEN, sizeof(char)); /* Return this array. */
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
       given. */

    if (nrhs == 0) {
	command_opts[0] = '\0';
	s[0] = '\0';
    }
    else if (nrhs == 1) {
	command_opts[0] = '\0';
	mxGetString(prhs[0], s, (MAX_URL_LEN-1));
	/* If the first char of `s' is `-' assume that s contains options 
           and since nrhs == 1, that no URL or filename was given. 
           
           I think this was possible when the code used to optionally read from
           a browser, but it is an error now given that the 'browser' option
           has been removed. 10/4/05 jhrg. */
	if (s[0] == '-') {
	    strncpy(command_opts, s, (MAX_STR-1));
	    command_opts[(MAX_STR-1)] = '\0';
	    s[0] = '\0';
	}
    }
    else if (nrhs == 2) {
	mxGetString(prhs[0], command_opts, (MAX_STR-1));
	mxGetString(prhs[1], s, (MAX_URL_LEN-1));
    }	
    else {			/* (nrhs > 2 || nrhs < 1) */
#if 0
	msg("usage: loaddap [options] [url | `*' | `?']\n");
#endif
	msg("usage: loaddap [options] url\n");
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

#if 0
    /*  We're omitting the communication w/browser. */
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
    char *dods_url = 0;
    char *dods_url_escaped = 0;
    char *pathname = 0;
    FILE *fin;
    
    /* This function is null for anything except linux. */
    lock_mex_function();

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

#if 0
    {
	int status;
	status = pclose(fin);
	if (status != 0) {
	    err_msg(\
"Error: The loaddap helper application writedap reported an error (%d). \n\
       This may be due to an earlier error. However, if there was no previous\n\
       error message, please report this to support@unidata.ucar.edu.\n",
                    status);
    }
#endif

        int status;
        status = pclose(fin);
        /* Added errno ECHILD for fc4 64-bit work-around */
        if (status != 0 && errno != ECHILD) {
            err_msg(\
"Error: Communication with the loaddap helper application writedap: \n\
%s\n", strerror(errno));
            err_msg(\
"This may be due to an earlier error. However, if there was no previous\n\
error message, please report this to support@unidata.ucar.edu.\n");
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

