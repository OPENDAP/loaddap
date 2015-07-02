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
   (c) COPYRIGHT URI 2000
   Please read the full copyright statement in the file COPYRIGHT.  

   Authors:
      jhrg,jimg		James Gallagher (jgallagher@gso.uri.edu)
*/

/** 
    A linked list class to manage Matlab mxArray variables that are to be
    interned as part of a structure. In all this software, assume that
    mxMalloc() and friends signal an out-of-memory error by exiting the mex
    program directly and returning to Matlab. Thus we never check the error
    status of these function calls.
*/

#include "config.h"

#include <mex.h>
#include <string.h>

#include "MLVars.h"

#if DMALLOC
// #include "dods_memory.h"
#include "dmalloc.h"
#define mxFree(x) dods_free(x)
#define mxCalloc(x, y) dods_calloc((x), (y))
#define mxMalloc(x) dods_malloc(x)
#define mxRealloc(x, y) dods_realloc((x), (y))
#endif

#include "defines.h"

MLVars *
init_ml_vars(void)
{
    MLVars *me = mxMalloc(sizeof(MLVars));
    me->_base = me->_end = me->_current = NULL;
    me->_length = 0;
    return me;
}

/* This function should free the nodes, but not the things they point to. */
void
clear_ml_vars(MLVars *ml_vars)
{
    _MLVar *next = NULL;
    _MLVar *node = NULL;

    mxAssert(ml_vars, "NULL ML Variable list");

    node = ml_vars->_base;
    while (node) {
	next = node->_next;
	mxFree(node);
	node = next;
    }

    mxFree(ml_vars);
}

void
add_ml_var(MLVars *ml_vars, mxArray *array, const char *vname)
{
    _MLVar *node = mxMalloc(sizeof(_MLVar));

    char *varName = mxMalloc(sizeof(char) * (strlen(vname)+1));
	mxAssert(ml_vars, "NULL ML Variable list");

    strcpy(varName, vname);

    /* Make the new node */
    node->_array = array;
    node->_vname = varName;
    node->_next = NULL;

    DBG2(msg("add_new_ml_var: %s, node: %d\n",varName,node));

    /* If the list is empty, this is the `base' */
    if (ml_vars->_base == NULL)
	ml_vars->_base = node;

    /* If the list is not empty, link this to the end */
    if (ml_vars->_end != NULL)
	ml_vars->_end->_next = node;

    /* Make this the end node */
    ml_vars->_end = node;

    /* Do bookkeeping */
    ml_vars->_length++;
    ml_vars->_current = node;
}

int
num_ml_var(MLVars *ml_vars)
{
    mxAssert(ml_vars, "NULL ML Variable list");
    return ml_vars->_length;
}

mxArray *
first_ml_var(MLVars *ml_vars)
{
    mxAssert(ml_vars, "NULL ML Variable list");
    ml_vars->_current = ml_vars->_base;
    return (ml_vars->_current) ? ml_vars->_current->_array : NULL;
}

mxArray *
next_ml_var(MLVars *ml_vars)
{
    mxAssert(ml_vars, "NULL ML Variable list");
    ml_vars->_current = ml_vars->_current->_next;
    return (ml_vars->_current) ? ml_vars->_current->_array : NULL;
}

mxArray *
current_ml_var(MLVars *ml_vars)
{
    mxAssert(ml_vars, "NULL ML Variable list");
    return (ml_vars->_current) ? ml_vars->_current->_array : NULL;
}

_MLVar *
get_current_ml_vars(MLVars *ml_vars)
{
     mxAssert(ml_vars, "NULL ML Variable list");
     return (ml_vars->_current);
}

char *
get_mxarray_name(_MLVar *ml_vars)
{
    mxAssert(ml_vars, "NULL ML Variable list");

    if (ml_vars != NULL)
        return ml_vars->_vname;
    else
	return 0;
}
