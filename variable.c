
/* 
   (c) COPYRIGHT URI/MIT 1996,1998,1999
   Please read the full copyright statement in the file COPYRIGHT.

   Authors:
      jhrg,jimg		James Gallagher (jgallagher@gso.uri.edu)
*/

/** Simple ADT for named vectors. These vectors are dynamically sized so that
    elements can be added one at a time without performance penalties. This is
    intended for use with the DODS/Matlab and DODS/IDL interface software. I
    wrote it because it appears that Matlab provides no easy and efficient
    way to build up vectors a single element at a time.

    Note that although vectors are added to a single value at a time storage
    is allocated in CLUMP bytes at a time. Thus many values can be added with
    only a few allocations.

    @author jhrg */

static char id[]={"$Id: variable.c,v 1.2 2003/12/08 17:59:50 edavis Exp $"};

#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#define MATLAB_MEX_FILE
#include <mex.h>

/** Allocate storage #CLUMP# elements at a time. */
#define CLUMP 1000

#if DMALLOC
// #include "dods_memory.h"
#include "dmalloc.h"
#define mxFree(x) dods_free(x)
#define mxCalloc(x, y) dods_calloc((x), (y))
#define mxMalloc(x) dods_malloc(x)
#define mxRealloc(x, y) dods_realloc((x), (y))
#endif

#include "defines.h"
#include "variable.h"

/** Search the list VARS for a variable named NAME. If found, return a pointer
    to the element, otherwise return NULL. 

    @see variable */

variable *
find_var(variable *vars, char *name)
{
    while (vars) {
	if (strcmp(vars->name, name) == 0)
	    return vars;
	else
	    vars = vars->next;
    }

    return NULL;
}

/** Add a variable named NAME to the linked list of variables VARS. Allocate
    enough memory to hold CLUMP elements and set the size of the vector to 0.
    The new element is added to the list; A pointer to the new element (and
    the new list head) is returned.

    @return A pointer to the new element. */

variable *
add_var(variable *vars, char *name, type t)
{
    variable *var;

    /* Make new element. */
    var = (variable *)mxMalloc(sizeof(variable));
    DBGM(fprintf(stderr, "mxMalloc (%s:%d): %x\n", __FILE__, __LINE__, var));
    var->name = (char *)mxMalloc(sizeof(char) * (strlen(name) + 1));
    DBGM(fprintf(stderr, "mxMalloc (%s:%d): %x\n", __FILE__, __LINE__, 
		 var->name));
    strcpy(var->name, name);
    var->type = t;
    var->length = 0;
    var->size = 0;
    var->max_string_len = 0;

    switch (t) {
      case float64:
	var->data = (double *)mxMalloc(sizeof(double) * CLUMP);
	var->size = CLUMP;
	DBGM(fprintf(stderr, "mxMalloc (%s:%d): %x\n", __FILE__, __LINE__, 
		     var->data));
	break;

      case string:
	var->sdata = (char **)mxMalloc(sizeof(char *) * CLUMP);
	var->size = CLUMP;
	DBGM(fprintf(stderr, "mxMalloc (%s:%d): %x\n", __FILE__, __LINE__, 
		     var->sdata));

	break;

      default:
	break;
    }
    
    mxAssertS(var->size == CLUMP, "");
    /* Attach the new element. */
    var->next = vars;

    return var;
}

/** Add #value# onto the end of the vector of values associated with the
    variable #var#.

    @return FALSE if #var# is NULL, TRUE otherwise. */

int
add_float64_value(variable *var, double value)
{
    if (!var || var->type != float64)
	return FALSE;

    mxAssertS(var->length <= var->size, "");
    if (var->length == var->size) {
	var->data = (double *)mxRealloc(var->data, 
				      sizeof(double) * (var->size + CLUMP));
	var->size += CLUMP;
	DBG2(fprintf(stderr, "mxRealloc: %x (%d bytes)\n", var->data, \
		     sizeof(double) * (var->size + CLUMP)));
    }
    
    mxAssertS(var->length < var->size, "");
    var->data[var->length++] = value;

    return TRUE;
}

/** Add #value# to the vector pointed to by #var#. Update #var#'s
    #max_string_length# field if this is the longest string added so far.

    @return FALSE if #var# is NULL, TRUE otherwise. */

int
add_string_value(variable *var, char *value)
{
    int string_len;

    if (!var || var->type != string)
	return FALSE;

    mxAssertS(var->length <= var->size, "");
    if (var->length == var->size) {
	var->sdata = (char **)mxRealloc(var->sdata, 
				      sizeof(char *) * (var->size + CLUMP));
	var->size += CLUMP;
	DBG2(fprintf(stderr, "mxRealloc: %x (%d bytes)\n", var->sdata, \
		     sizeof(char *) * (var->size + CLUMP)));
    }

    string_len = strlen(value);
    if (string_len > var->max_string_len)
	var->max_string_len = string_len;

    mxAssertS(var->length < var->size, "");
    var->sdata[var->length] = (char *)mxMalloc(string_len + 1);
    DBGM(fprintf(stderr, "mxMalloc (%s:%d): %x\n", __FILE__, __LINE__, 
		 var->sdata[var->length]));

    strcpy(var->sdata[var->length++], value);

    return TRUE;
}

/**
   Free the list of variables pointed to by VARS. */

void
free_vars(variable *vars)
{
    variable *v = vars;

    if (!vars)
	return;

    do {
	vars = vars->next;
	DBGM(fprintf(stderr, "mxFree (%s:%d): %x\n", __FILE__, __LINE__, 
		     v->name));
	mxFree(v->name);
	/* Don't free data (but I don't understand why). 2/18/2000 jhrg */

	DBGM(fprintf(stderr, "mxFree (%s:%d): %x\n", __FILE__, __LINE__, v));
	mxFree(v);
	v = vars;
    } while (vars);
}

/* 
 * $Log: variable.c,v $
 * Revision 1.2  2003/12/08 17:59:50  edavis
 * Merge release-3-4 into trunk
 *
 * Revision 1.1.1.1  2003/10/22 19:43:35  dan
 * Version of the Matlab CommandLine client which uses Matlab Structure
 * variables to maintain the shape of the underlying DODS data.
 *
 * Revision 1.17  2003/04/22 14:47:25  dan
 * Removed changes added to maintain DDS structure, these
 * changes have been placed on the structure-format1 branch
 * until the GUI is ready to use them.
 *
 * Revision 1.15  2001/08/27 18:06:57  jimg
 * Merged release-3-2-5.
 *
 * Revision 1.14.2.1  2000/12/13 01:33:31  jimg
 * Added defines.h or moved it after other preprocessor lines
 *
 * Revision 1.14  2000/11/22 23:43:00  jimg
 * Merge with pre-release 3.2
 *
 * Revision 1.9.8.7  2000/09/22 20:54:04  jimg
 * Added dmalloc stuff in DMALLOC guard
 *
 * Revision 1.13  2000/07/21 10:21:56  rmorris
 * Merged with win32-mlclient-branch.
 *
 * Revision 1.11.2.1  2000/06/26 23:03:39  rmorris
 * Mods for port to win32.
 *
 * Revision 1.12  2000/06/20 21:40:09  jimg
 * Merged with 3.1.6
 *
 * Revision 1.9.8.6  2000/06/20 20:36:46  jimg
 * Imporved error messages and fixed some doc++ comments (although some
 * problems remain).
 *
 * Revision 1.11  2000/04/20 23:38:13  jimg
 * Merged with release 3.1.3
 *
 * Revision 1.9.8.5  2000/04/20 19:45:15  jimg
 * Changed some int variables to size_t.
 *
 * Revision 1.9.8.4  2000/04/19 05:40:28  jimg
 * Added conditional efence malloc defines.
 *
 * Revision 1.9.8.3  2000/04/18 22:34:54  jimg
 * Added assert calls.
 * Fixed a bug in the allocation of memory when adding to a vector. This fixed
 * used the new `size' field. Before length was compared to CLUMP, not the
 * number of elements currently allocated (what size holds).
 *
 * Revision 1.10  2000/04/01 01:16:24  jimg
 * Merged with release-3-1-2
 *
 * Revision 1.9.8.2  2000/03/28 23:59:39  jimg
 * Removed the freeing of data since this causes dangling pointers (I don't
 * understand why that's the case since I thought all those *values* were
 * copied...). Also, remove the reorganize_strings() function since strings are
 * now interned using the new ML string array.
 *
 * Revision 1.9.8.1  2000/02/19 00:36:15  jimg
 * Switched from the ML 4 to 5 API.
 *
 * Revision 1.9  1999/04/30 17:06:58  jimg
 * Merged with no-gnu and release-2-24
 *
 * Revision 1.8.14.1  1999/04/29 18:48:30  jimg
 * Fixed comments and copyright.
 *
 * Revision 1.8  1998/09/16 23:03:43  jimg
 * Fixed the mxCalloc/allocator prototype.
 *
 * Revision 1.7  1998/03/21 00:18:58  jimg
 * Added some documentation.
 * Added reorganize_strings() which simplifies interning vectors of strings.
 *
 * Revision 1.6  1998/03/20 00:34:09  jimg
 * Moved many defines to a new header file: defines.h
 *
 * Revision 1.5  1997/12/16 01:01:28  jimg
 * Fixed bug where max_string_len was set to an incorrect value.
 * Moved structure definition to header file.
 *
 * Revision 1.4  1996/12/18 00:51:32  jimg
 * Added include for dmalloc.
 *
 * Revision 1.3  1996/10/29 21:44:43  jimg
 * Added string.h to included files
 *
 * Revision 1.2  1996/10/25 23:04:41  jimg
 * Added debugging statements for malloc/realloc/free.
 * Added support for string vectors.
 * Removed assert.h since that breaks compilation with gcc - gcc will reference
 * `__eprintf' which cmex cannot find.
 *
 * Revision 1.1  1996/10/23 23:55:06  jimg
 * Created.
 */
