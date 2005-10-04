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

#ifdef __cplusplus
extern "C" {
#endif

    extern int count_elements(const int ndims, const int dims[]);
    extern int remove_null_axis(const int ndims, int dims[]);

    extern char *get_variable_name(int first);
    extern void clear_existing_variables();
    extern int get_number_of_variables();

    extern void cm2rm(double *dest, double *src, int rows, int cols);
    extern void rm2cm(double *dest, double *src, int rows, int cols);

    extern int intern(const char *name, int ndims, int dims[], double *yp, 
		      int extend);
    extern int intern_strings(const char *name, int m, char **s, int extend, mxArray **array_ptr);

    extern mxArray *build_var(const char *name, const int ndims, 
			      const int dims[], double *yp);
    extern mxArray *build_string_var(const char *name, int m, char **s);

    extern MLVars *transfer_variables(const char *lName, MLVars *vars);

    extern mxArray *build_ml_vars(const char *name, MLVars *vars);

#ifdef __cplusplus
}
#endif
