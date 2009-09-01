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

#ifndef _mlvars_h
#define _mlvars_h

typedef struct _MLVar _MLVar;

struct _MLVar {
    /** The Matlab array variable */
    mxArray *_array;
    /** The Matlab array variable name */
    char *_vname;
    /** Pointer to the next variable */
    _MLVar *_next;
};

typedef struct MLVars {
    /** Pointer to the start of the list. */
    _MLVar *_base;
    /** Pointer to the end of the list. */
    _MLVar *_end;
    /** Pointer to the current element (changed by first\_ml\_var() and
	next\_ml\_var()). */
    _MLVar *_current;
    /** NUmber of elements in the list. */
    int _length;
} MLVars;

extern MLVars *init_ml_vars(void);
extern void clear_ml_vars(MLVars *ml_vars);
extern void add_ml_var(MLVars *ml_vars, mxArray *array, const char *vname);
extern int num_ml_var(MLVars *ml_vars);
extern mxArray *first_ml_var(MLVars *ml_vars);
extern mxArray *next_ml_var(MLVars *ml_vars);
extern mxArray *current_ml_var(MLVars *ml_vars);

extern _MLVar *get_current_ml_vars(MLVars *ml_vars);
extern char *get_mxarray_name(_MLVar *ml_vars);

#endif /* _mlvars_h */
