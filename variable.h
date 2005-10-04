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
   (c) COPYRIGHT URI/MIT 1996,1998
   Please read the full copyright statement in the file COPYRIGHT.

   Authors:
      jhrg,jimg		James Gallagher (jgallagher@gso.uri.edu)
*/

/** #type# is used to denote the type of an instance of variable.

    @see variable. */

typedef enum type {
    float64,
    string
} type;

/* Note that `max_string_len' is undefined when `type' is not string. I added
   this field so that arrays of strings can `easily' be interned in Matlab.
   jhrg 10/31/97 */

/** #variable# holds one element of list of `built vectors'. */ 
typedef struct var {
    /** The name of the variable. */
    char *name;
    /** Number of elements in this vector. */
    int length;
    /** Amount of memory allocated to this vector. The units of size are
	either flost64s or char *s. */
    int size;
    /** Type of data in this vector
	@see type. */
    enum type type;
    /** If #type# is #string#, this holds the length of the longest string in
	the vector. */
    int max_string_len;
    /** If #type# is string this is a pointer to the string data. */
    char **sdata;
    /** If #type# is double this is a pointer to the float data. */
    double *data;
    struct var *next;
} variable;

variable *find_var(variable *vars, char *name);
variable *add_var(variable *vars, char *name, type t);
int add_float64_value(variable *var, double value);
int add_string_value(variable *var, char *value);
void free_vars(variable *vars);

