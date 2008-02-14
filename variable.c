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

static char id[]={"$Id$"};

#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

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
    DBGM(msg("mxMalloc (%s:%d): %x\n", __FILE__, __LINE__, var));
    var->name = (char *)mxMalloc(sizeof(char) * (strlen(name) + 1));
    DBGM(msg("mxMalloc (%s:%d): %x\n", __FILE__, __LINE__, 
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
	DBGM(msg("mxMalloc (%s:%d): %x\n", __FILE__, __LINE__, var->data));
	break;

      case string:
	var->sdata = (char **)mxMalloc(sizeof(char *) * CLUMP);
	var->size = CLUMP;
	DBGM(msg("mxMalloc (%s:%d): %x\n", __FILE__, __LINE__, var->sdata));

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
	DBG2(msg("mxRealloc: %x (%d bytes)\n", var->data, \
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
	DBG2(msg("mxRealloc: %x (%d bytes)\n", var->sdata, \
		     sizeof(char *) * (var->size + CLUMP)));
    }

    string_len = strlen(value);
    if (string_len > var->max_string_len)
	var->max_string_len = string_len;

    mxAssertS(var->length < var->size, "");
    var->sdata[var->length] = (char *)mxMalloc(string_len + 1);
    DBGM(msg("mxMalloc (%s:%d): %x\n", __FILE__, __LINE__, 
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
	DBGM(msg("mxFree (%s:%d): %x\n", __FILE__, __LINE__, v->name));
	mxFree(v->name);
	/* Don't free data (but I don't understand why). 2/18/2000 jhrg */

	DBGM(msg("mxFree (%s:%d): %x\n", __FILE__, __LINE__, v));
	mxFree(v);
	v = vars;
    } while (vars);
}
