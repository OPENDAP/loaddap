
/* 
   (c) COPYRIGHT URI/MIT 1997,1998,1999,2000
   Please read the full copyright statement in the file COPYRIGHT.  

   Authors:
      jhrg,jimg		James Gallagher (jgallagher@gso.uri.edu)
*/

/**
   This file contains the software used to intern values and to combine new
   values with those already interned.

   Extend a Matlab vector/matrix by combining an existing array with new
   data. Combining two vectors of length M and N yields a vector of length M
   + N. Combining two matrices of size r1 x c1 and r2 x c2 yields a matrix of
   size r1 x c1 + c2 and presumes that r1 == r2.

   @author jhrg 3/5/97 
*/

static char id[]={"$Id$"};

#include <stdio.h>
#include <string.h>

#include <mex.h>

#if DMALLOC
// #include "dods_memory.h"
#include "dmalloc.h"
#define mxFree(x) dods_free(x)
#define mxCalloc(x, y) dods_calloc((x), (y))
#define mxMalloc(x) dods_malloc(x)
#define mxRealloc(x, y) dods_realloc((x), (y))
#endif

#include "defines.h"
#include "MLVars.h"
#include "error.h"

#ifdef TEST
static mxArray **return_args = NULL;
static int num_return_args = 0;
#else
extern mxArray **return_args;	/* see loaddods.cc */
extern int num_return_args;
#endif

extern int current_arg;		/* defined in loaddods.c */

#define ISVECTOR(r, c) ((r) == 1 || (c) == 1)
#define MATRICES_MATCH(r1, c1, r2, c2) ((r1) == (r2) || (c1) == (c2))

/** Given a dimension vector and the number of dimensions, remove any
    dimensions whose size is 1. */
int 
remove_null_axis(const int ndims, int dims[])
{
    int i, j;
    for (i = 0, j = 0; i < ndims; i++) {
	if (dims[i] > 1) {
	    dims[j++] = dims[i];
	}
    }

    return j;
}

/** Given a dimension vector and the number of dimensions, return the total
    number of elements in the array they describe. */
int 
count_elements(const int ndims, const int dims[])
{
    int i, total = 1;
    for (i = 0; i < ndims; i++)
	total *= dims[i];

    return total;
}

/** Transform the row-major order data that DODS sends to column-major order
   data for Matlab.

   C's row-mayor ordering uses a different index/dimension order than
   Matlab's N-mayor ordering, in addition to organizing the information
   differently in memory. We need functions to transform the C's
   representation of dimensions into Matlab's.

@{
*/

/** Given a current index tuple and the dimensionality of the data, compute 
    the next tuple. Vary the rightmost element of the tuple the fastest.
    Return true if there *is* a next tuple (which is passed back in
    current_index), false otherwise.
*/

static int
get_next_rm_index(int current_index[], const int ndims, const int dims[])
{
    int i;
    for (i = ndims-1; i >= 0; --i) {
	current_index[i]++;
	if (current_index[i] < dims[i])
	    return TRUE;
	else
	    current_index[i] = 0;
    }

    return FALSE;
}

/** Given an index into, and the dimensions of the array, return the offset
    needed to access the referenced element assuming row-major storage order.
*/

static int
get_rm_offset(int current_index[], const int ndims, const int dims[])
{
    int offset = 0;
    int j, k, t;
    for (j = 0; j < ndims; ++j) {
	t = 1;
	for (k = j+1; k < ndims; ++k)
	    t *= dims[k];
	offset += current_index[j] * t;
    }

    return offset;
}

/** Given an index into, and the dimensions of the array, return the offset
    needed to access the referenced element assuming column-major storage
    order. */

static int
get_cm_offset(int current_index[], const int ndims, const int dims[])
{
    int offset = 0;
    int j, k, t;
    for (j = 0; j < ndims; ++j) {
	t = 1;
	for (k = 0; k < j; ++k)
	    t *= dims[k];
	offset += current_index[j] * t;
    }

    return offset;
}

/** Given the double array described by dims[ndims], transform that array
    from column-major to row-major order. Put the result in dest. 

    NB: ndims must not be > than 2. Because of that constraint there is no
    need to use a cm-ordered set of dimensions and indices.

    This function exists solely to support the old -k (concatenation) option
    which should not be used when ndims is > than 2 since it becomes
    ambiguous just how to assemble the separate data values. This is better
    done by the user in Matlab once they have the data. For example, use the
    return value feature of loaddods and build up the n-dim aggregate object
    in a loop. */
static void
cm2rm_limited(double *dest, double *src, const int ndims, const int dims[])
{
    int *index = (int *)mxCalloc(ndims, sizeof(int));

    if (ndims > 2) {
	err_msg(
"Internal Error: cm2rm_limited does not support ndims > 2 (%s:%d)\n",
		__FILE__, __LINE__);
	return;
    }

    do {
	int rm_offset = get_rm_offset(index, ndims, dims);
	int nm_offset = get_cm_offset(index, ndims, dims);

	*(dest + rm_offset) = *(src + nm_offset);
    } while (get_next_rm_index(index, ndims, dims));

    mxFree(index);
}

/** Given the row-major index array (with ndims-1 elements), load values into
    cm_dims so that the dimensions will be listed in Matlab's n-major order. 

    Assume that enough storage has already been allocated to cm_dims. 

    Algorithm: RM order: Plane, Row,   Column
               NM order: Row,   Column, Plane.

    Move the last two dimensions from the RM order to the first two of the NM
    order. Then copy each plane dimension from the front of the RM ordering
    to the back of the NM ordering reversing those entries as they are
    copied. 

    Note that if ndims is less then three then this function just copies the
    values and, in that case, it is better to not use this function. */
static void
rm_index2cm_index(int cm_dims[], const int rm_dims[], const int ndims)
{
    switch(ndims) {
      case 1:
	cm_dims[0] = rm_dims[0];
	break;
	
      case 2:
	cm_dims[0] = rm_dims[0];
	cm_dims[1] = rm_dims[1];
	break;

      default: 
	{		/* ndims >= 3 */
	    int i, j;
	    cm_dims[0] = rm_dims[ndims-2];
	    cm_dims[1] = rm_dims[ndims-1];
    
	    /* Copy and reverse the plane (or page) entries */
	    /* What this loop does: P0 P1 P2 R C
	                              <--  ^
             	                           | i
	       Start i at P2 and move it towards the front of the rm_dims
	       array. Tack each plane index P* onto the end of cm_dims so
	       that the final ordering looks like: R C P2 P1 P0 */
	    j = 2;
	    for (i = ndims-3; i >= 0; i--)
		cm_dims[j++] = rm_dims[i];
	    break;
	}
    }
}

/** Given the double array described by rm_dims[ndims], transform that array
    from row-major to column-major order. Put the result in dest.

    Assume that the rm_dims array contains the dimension information in
    row-major order.
@}*/
void
rm2cm(double *dest, double *src, const int ndims, const int rm_dims[])
{
    int *cm_index;
    int *rm_index;
    int *cm_dims;
    int rm_offset, cm_offset;

    if (ndims <= 2) {
	/* An optimization: when ndims is 0, 1 or 2 then the rm and cm index
	   order is the same. Only transform the indices when ndims is > 2. */
	rm_index = (int *)mxCalloc(ndims, sizeof(int));
	do {
	    rm_offset = get_rm_offset(rm_index, ndims, rm_dims);
	    cm_offset = get_cm_offset(rm_index, ndims, rm_dims);

	    *(dest + cm_offset) = *(src + rm_offset);
	} while (get_next_rm_index(rm_index, ndims, rm_dims));

	mxFree(rm_index);
    }
    else {
	cm_index = (int *)mxCalloc(ndims, sizeof(int));
	rm_index = (int *)mxCalloc(ndims, sizeof(int));
	cm_dims = (int *)mxCalloc(ndims, sizeof(int));
	rm_index2cm_index(cm_dims, rm_dims, ndims);

	do {
	    rm_offset = get_rm_offset(rm_index, ndims, rm_dims);
	    rm_index2cm_index(cm_index, rm_index, ndims);
	    cm_offset = get_cm_offset(cm_index, ndims, cm_dims);

	    *(dest + cm_offset) = *(src + rm_offset);
	} while (get_next_rm_index(rm_index, ndims, rm_dims));

	mxFree(cm_index);
	mxFree(rm_index);
	mxFree(cm_dims);
    }
}

/** Extend the Matlab variable #name# by adding the elements referenced by
    #pr2#. The data referred to by #pr2# may be a scalar, vector or matrix.
    However, its shape must match that of #name#'s in at least one dimension.
    To extend #name#, the values in #pr2# will be added by catenating so that
    the common dimension retains its size (e.g., if #name# is a 2 x 3 matrix
    and #pr2# is a 2 x 4, then the result will be a 2 x 7). If both the row
    and column sizes are the same #name# will be extended by catenating rows.
    The shape of #name# determines the shape of the resulting vector or
    matrix.

    NB: This is only visible within this file.

    NB: ndims must be either 1 or 2. 

    @param name The name of the variable to extend (this must already exist
    in the Matlab workspace.
    @param pr2 A reference to data to be added to #name#.
    @param ndims The number of dimensions in #pr2#.
    @param dims The size of each of the dimensions in #pr2#. */

static int
extend_array(const char *name, const int ndims, const int dims[], double *pr2)
{
    int r1, c1, r2, c2;
    double *pr1, *pr;
    int s1, s2;
    int r, c;
    const mxArray *m1 = mexGetArrayPtr(name, "caller");
    mxArray *m2;

    if (!m1) {
	err_msg(
"Internal Error: Could not find the array `%s' (%s:%d)\n", name, __FILE__, 
                __LINE__);
	return FALSE;
    }

    switch (ndims) {
      case 0:
	r2 = 1;
	c2 = 1;
	break;
      case 1:
	r2 = dims[0];
	c2 = 1;
	break;
      case 2:
	r2 = dims[0];
	c2 = dims[1];
	break;
      default:
	if (ndims > 2)
           err_msg(
"Error: loaddods cannot extend arrays that have more than two dimensions.\
\nSee `help loaddods' for more information.");
	else 
	    err_msg(
"Internal Error: Bad value for dimension count (%s:%d)\n", __FILE__, __LINE__);
	return FALSE;
    }

    r1 = mxGetM(m1);		/* Get the row count of Matrix `m1' */
    c1 = mxGetN(m1);		/* And column count */
    pr1 = mxGetPr(m1);		/* And pointer to real data */
	
    if (!pr1) {
	err_msg(
"Internal Error: No real-valued data in the array `%s' (%s:%d)\n", name, __FILE__, __LINE__);
	return FALSE;
    }

    DBG2(fprintf(stderr, "m1: %x, r1: %d, c1: %d, pr1: %x\n", m1, r1, c1,\
		 pr1));
    
    if (ISVECTOR(r1, c1) && ISVECTOR(r2, c2)) {
	s1 = MAX(r1, c1);
	s2 = MAX(r2, c2);
	/* Use the `shape' of `m1' to determine the shape of the extended
	   vector. */
	r = (c1 == 1) ? s1 + s2 : 1;
	c = (c1 != 1) ? s1 + s2 : 1;
	m2 = mxCreateDoubleMatrix(r, c, mxREAL); /* free with mxDestroyArray */

	pr = (double *)mxCalloc(s1 + s2, sizeof(double));
	DBGM(fprintf(stderr, "mxCalloc (%s:%d): %x\n", __FILE__, __LINE__, 
		     pr));

	/* Copy old and new data */
	memcpy(pr, pr1, s1 * sizeof(double));
	memcpy(pr + s1, pr2, s2 * sizeof(double));

    }
    else if (MATRICES_MATCH(r1, c1, r2, c2)) {
	s1 = r1 * c1;
	s2 = r2 * c2;
	c = (c1 == c2) ? c1 : c1 + c2;
	r = (c1 == c2) ? r1 + r2 : r1;
	m2 = mxCreateDoubleMatrix(r, c, mxREAL);

	pr = (double *)mxCalloc(s1 + s2, sizeof(double));
	DBGM(fprintf(stderr, "mxCalloc (%s:%d): %x\n", __FILE__, __LINE__, 
		     pr));
	
	if (c1 == c2) {
	    /* When columns are equal, extend by catenating rows. Given that
	       the Matlab array is already in column major order and the new
	       data is in row major order, the operation proceeds as:
	       1) Transform the Matlab data to row major order.
	       2) catenate the Matlab and DODS data.
	       3) transform the result back to column major order.

	       E.G.: A is the original Matlab data, B is the new information.
	             A = 1 2 3            B = 7 8 9
		         4 5 6
		     
		     a = 1 4 2 5 3 6      b = 7 8 9

		     step 1: a' = 1 2 3 4 5 6
		          2: c = a' b = 1 2 3 4 5 6 7 8 9
			  3  c' = 1 4 7 2 5 8 3 6 9
			  
		     Which yields: C = 1 2 3
		                       4 5 6
				       7 8 9
	    */
            int *new_dims = (int *) mxMalloc( ndims * sizeof(int));
	    double *prp;
	    double *pr1p = (double *)mxCalloc(r1 * c1, sizeof(double));
            DBGM(fprintf(stderr, "mxMalloc (%s:%d): %x\n", __FILE__,
                         __LINE__, new_dims));
	    DBGM(fprintf(stderr, "mxCalloc (%s:%d): %x\n", __FILE__, 
			 __LINE__, pr1p));
	    prp = (double *)mxCalloc(r * c, sizeof(double));
	    DBGM(fprintf(stderr, "mxCalloc (%s:%d): %x\n", __FILE__, 
			 __LINE__, prp));

	    cm2rm_limited(pr1p, pr1, ndims, dims);

	    memcpy(pr, pr1p, s1 * sizeof(double));
	    memcpy(pr + s1, pr2, s2 * sizeof(double));
	    
	    /* We know that ndims is 2 because this function is limited to
	       Vectors and Matrices and this is the Matrix section. */
	    new_dims[0] = r;
	    new_dims[1] = c;
	    rm2cm(prp, pr, ndims, new_dims);

            DBGM(fprintf(stderr, "mxFree (%s:%d): %x\n", __FILE__, __LINE__, 
                         new_dims));
            mxFree( new_dims);
	    DBGM(fprintf(stderr, "mxFree (%s:%d): %x\n", __FILE__, __LINE__, 
			 pr));
	    mxFree(pr);
	    DBGM(fprintf(stderr, "mxFree (%s:%d): %x\n", __FILE__, __LINE__, 
			 pr1p));
	    mxFree(pr1p);
	    /* Don't free pr1 since Matlab manages that memory */

	    pr = prp;		/* ...so that mxSetPr() further down works */
	}
	else {
	    /* When rows are equal, extend by catenating columns. Do this by
	       transforming the row major data to column major and appending.
	       E.G.: A = 1 2 3        B = 7 8
	                 4 5 6            9 10

		     a = 1 4 2 5 3 6  b' = 7 9 8 10

		     c = 1 4 2 5 3 6 7 9 8 10
		      
		     C = 1 2 3 7 8
		         4 5 6 9 10
	    */
	    double *pr2p = (double *)mxCalloc(r2 * c2, sizeof(double));
	    DBGM(fprintf(stderr, "mxCalloc (%s:%d): %x\n", __FILE__, 
			 __LINE__, pr2p));

	    rm2cm(pr2p, pr2, ndims, dims);
	    
	    memcpy(pr, pr1, s1 * sizeof(double));
	    memcpy(pr + s1, pr2p, s2 * sizeof(double));
	    
	    DBGM(fprintf(stderr, "mxFree (%s:%d): %x\n", __FILE__, __LINE__, 
			 pr2p));
	    mxFree(pr2p);
	}

	DBG2(fprintf(stderr, "matrix:\n"));
    }
    else {
	err_msg(
"Error: You can only combine variables which share a common dimension.\
\nSee `help loaddods' for more information.\n");
	return FALSE;
    }

    DBG2(fprintf(stderr, "s1: %d, s2: %d\n", s1, s2));
    DBG2(fprintf(stderr, "r: %d, c: %d\n", r, c));

    /* set name and data of new matrix */
    mxSetPr(m2, pr);
    mxSetPi(m2, 0);
    mxSetName(m2, name);

    /* Intern */
    mexPutArray(m2, "caller");

    return TRUE;
}

/** Given the name of a vector of strings append the #m# strings data pointed
    to by #s# to it. If no such vector exists, return FALSE, otherwise return
    TRUE. */

static int
extend_strings(const char *name, int m, char **s)
{
    const mxArray *mp = mexGetArrayPtr(name, "caller");
    mxArray *array_ptr;
    int rows = mxGetM(mp);
    int max_len = mxGetN(mp);
    char **new_strings;
    int status;
    int i;
    char *orig_strings = mxArrayToString(mp);
    DBGM(fprintf(stderr, "mxMalloc (%s:%d): %x\n", __FILE__, __LINE__, 
		 orig_strings));
    
    new_strings = (char **)mxMalloc((rows + m) * sizeof(char *));

    /* Wire up the first set of pointers; pointers to the original data.
       Don't copy the data, just copy the pointers. Note that orig_strings is
       one long array; strings are padded with extra NULLs to make the matrix
       rectangular. 2/18/2000 jhrg */
    for (i = 0; i < rows; ++i) 
	*(new_strings + i) = orig_strings + (i * max_len);

    /* Now add pointers to the new data. Note that s is an array of pointers
       to char like new_strings. 2/18/2000 jhrg*/
    for (i = rows; i < rows + m; ++i)
	*(new_strings + i) = *(s + m);
    
    array_ptr = mxCreateCharMatrixFromStrings(rows + m, 
					      (const char **)new_strings);
    mxSetName(array_ptr, name);
    status = (mexPutArray(array_ptr, "caller") == 0);

    return status;
}

/**
  Maintain a list of variables which are new (to this invocation of loaddods)
  so that we know when to clobber something that exists in Matlab versus when
  to extend. The rule is, extend if is_existing_variable() returns true,
  otherwise, don't extend but do call add_new_variable(). Of course, if the
  user does not want `extension' don't do any of this stuff.

  NB: This is only visible within this file.
*/

static struct {
    int len;			/* How many variables are recorded here? */
    int size;			/* How much space is allocated for vars */
    char **name;		/* Array of strings for names. */
} variables = {0, 0, 0};

/** Add the name #name# to the list of new variables read during this run of
    loaddods. 

    NB: This is only visible within this file. */

static void
add_new_variable(const char *name)
{
    if (variables.len == variables.size) {
	variables.size += 20;
	if (variables.len == 0) {
	    variables.name = (char **)mxMalloc(variables.size 
					       * sizeof(char *));
	    DBGM(fprintf(stderr, "mxMalloc (%s:%d): %x\n", __FILE__, 
			 __LINE__, variables.name));
	}
	else {
	    variables.name = (char **)mxRealloc(variables.name,
						variables.size 
						* sizeof(char *));
	    DBGM(fprintf(stderr, "mxRealloc (%s:%d): %x\n", __FILE__, 
			 __LINE__, variables.name));
	}
    }

    variables.name[variables.len] = (char *)mxMalloc(strlen(name)+1);
    DBGM(fprintf(stderr, "mxMalloc (%s:%d): %x\n", __FILE__, __LINE__, 
		 variables.name[variables.len]));
    strcpy(variables.name[variables.len++], name);
}

/** Wipe clean the list of variable names read so far by loaddods. Normally
    this would be run at the start of each run of loaddods.  Rely on the
    mxMalloc, etc. to clean up the memory allocated here. */

void
clear_existing_variables()
{
    while (variables.len > 0) {
	variables.len--;
	DBGM(fprintf(stderr, "mxFree (%s:%d): %x\n", __FILE__, __LINE__,
		     variables.name[variables.len]));
	mxFree(variables.name[variables.len]);
    }
    if (variables.size > 0) {
	DBGM(fprintf(stderr, "mxFree (%s:%d): %x\n", __FILE__, __LINE__,
		     variables.name));
	mxFree(variables.name);
    }

    variables.len = 0;
    variables.size = 0;
    variables.name = NULL;
}

/** Is the variable #name# in the set of variables read during this
    invocation of loaddods? 

    NB: This is only visible within this file.

    @return TRUE if #name# is on the list of current variables, FALSE
    otherwise. */

static int
is_existing_variable(const char *name)
{
    int i = 0;
    while (i < variables.len)
	if (strcmp(name, variables.name[i++]) == 0)
	    return TRUE;

    return FALSE;
}

#if 0
static char *
enforce_name_limits(char *name)
{
    if (truncate_variable_names && (strlen(name) > 19))
	name[18] = '\0';

    return name;
}
#endif

/** Return the number of variables currently in the list. */

int
get_number_of_variables()
{
    return variables.len;
}

/** Return the next name in the list of variable names. Return NULL when there
    are no more names. If FIRST is true, start over from the front of the
    list. */

char *
get_variable_name(int first)
{
    static int item = 0;

    if (first)
	item = 0;

    if (item >= variables.len)
	return NULL;
    else
	return variables.name[item++];
}

/** 
   Create a mxArray for the named variable.

   @param name The new array's name.
   @param ndims The number of dimensions of the array.
   @param dims The dimension sizes of the array.
   @param yp 64-bit floating point values for the array.
*/
mxArray *
build_var(const char *name, const int ndims, const int dims[], double *yp)
{
    mxArray *array_ptr;
    switch (ndims) {
      case 0:
	array_ptr = mxCreateDoubleMatrix(1, 1, mxREAL);
	break;
      case 1:
	array_ptr = mxCreateDoubleMatrix(dims[0], 1, mxREAL);
	break;
      case 2:
	array_ptr = mxCreateDoubleMatrix(dims[0], dims[1], mxREAL);
	break;
      default: 
	  {
	      int *cm_dims = (int *)mxCalloc(ndims, sizeof(int));
	      rm_index2cm_index(cm_dims, dims, ndims);
	      array_ptr = mxCreateNumericArray(ndims, cm_dims, 
					       mxDOUBLE_CLASS, mxREAL);
	      mxFree(cm_dims);
	  }
	break;
    }

    rm2cm(mxGetPr(array_ptr), yp, ndims, dims);
    mxSetName(array_ptr, name);

    return array_ptr;
}

/**
   Add into the Matlab workspace a Float variable #name# with values #yp#.
   The variable may be a scalar, vector or matrix - this is determined by the
   #ndims# and #dims# parameters. If #extend# is true, then if a variable
   with #name# already exists in the workspace add these values to it rather
   than replacing it with these values.

   NB: When using the return arguments feature of loaddods #extend# has no
   effect (because return args always creates new values and passes them back
   to the caller instead of interning them directly in the Matlab workspace.

   @param name The name of the new variable.
   @param ndims The number of dimensions of the variable. This will be used
   as the size of the #dims# parameter.
   @param dims An array of dimension sizes.
   @param yp A pointer to the data in N-Major order.
   @return TRUE if successful, FALSE otherwise. */
   
int
intern(const char *name, int ndims, int dims[], double *yp, int extend)
{
    mxArray *array_ptr;
    int exists;
    int status;

    /* Perform bookkeeping for the extend option. Every time a new variable
       name appears, record that name. Only variables with the same name can
       be concatenated (extended). 2/18/2000 jhrg */
    exists = is_existing_variable(name);
    if (!exists)
	add_new_variable(name);
    
    /* Using the `extend' option and using return arguments is mutually
       exclusive. The return argument syntax overrides using the -k option. */
    if (num_return_args) {
	if (current_arg+1 > num_return_args)
	    return TRUE;	/* not an error, technically */

	return_args[current_arg] = build_var(name, ndims, dims, yp);

	current_arg++;
	return TRUE;
    }

    if (extend && exists) {
	if (!extend_array(name, ndims, dims, yp)) {
	    err_msg(
"Internal Error: Could not create variable for `%s' (%s:%d)\n", name,
		    __FILE__, __LINE__);
	    return FALSE;
	}
	else
	    return TRUE;
    }

    array_ptr = build_var(name, ndims, dims, yp);

    status = (mexPutArray(array_ptr, "caller") == 0);
    if (!status) 
	err_msg("Internal Error: Could not intern array %s.\n", name);
    return status;
}

int
create_vector(const char *name, int ndims, int dims[], double *yp, int extend, mxArray **array_ptr)
{
    int exists;
    int status;

    /* Perform bookkeeping for the extend option. Every time a new variable
       name appears, record that name. Only variables with the same name can
       be concatenated (extended). 2/18/2000 jhrg */
    exists = is_existing_variable(name);
    if (!exists)
	add_new_variable(name);
    
    /* Using the `extend' option and using return arguments is mutually
       exclusive. The return argument syntax overrides using the -k option. */
    if (num_return_args) {
	if (current_arg+1 > num_return_args)
	    return TRUE;	/* not an error, technically */

	return_args[current_arg] = build_var(name, ndims, dims, yp);

	current_arg++;
	return TRUE;
    }

    if (extend && exists) {
	if (!extend_array(name, ndims, dims, yp)) {
	    err_msg(
"Internal Error: Could not create variable for `%s' (%s:%d)\n", name,
		    __FILE__, __LINE__);
	    return FALSE;
	}
	else
	    return TRUE;
    }

    *array_ptr = build_var(name, ndims, dims, yp);
    
    status = 1;

    if (!status) 
	err_msg("Internal Error: Could not intern array %s.\n", name);
    return status;
}

/**
   Read through the MLVars list adding each mxArray to a structure called
   #name#.

   @param name The name of the new ML Structure.
   @param vars A linked list of Matlab mxArrays that are to be the fields of
   the new structure.
*/

mxArray *
build_ml_vars(const char *name, MLVars *vars)
{
    int num_vars = num_ml_var(vars);
    const char **names = mxMalloc(num_vars * sizeof(char *));
    mxArray *ret_array;

    /* Allocate the new ML structure. We must gather the names of the
       structure's fields *before* creating the structure. */
    {

	int i = 0;
	mxArray *v = first_ml_var(vars);
	while (v) {
	    names[i++] = mxGetName(v);
	    DBG2(msg("build_ml_vars: name[%d]: %s\n",i-1,mxGetName(v)));
	    v = next_ml_var(vars);
	}
    }

    DBG2(msg("build_ml_vars: %d\n",vars));
    ret_array = mxCreateStructMatrix(1, 1, num_vars, names);
    mxSetName(ret_array, name);

    /*iterate over the vars list adding each mxArray */
    {
	int i = 0;
	mxArray *v = first_ml_var(vars);
	while (v) {
	    mxSetFieldByNumber(ret_array, 0, i++, v);
	    v = next_ml_var(vars);
	}
    }

    return ret_array;
}

MLVars *
transfer_variables(const char *lName, MLVars *vars)
{
    int status, nFields, idx, i, j;
    int numUniqueVariables;
    int start, next, count;
    bool Found = false;
    const char *name;
    MLVars *structArray;
    mxArray *ret_array;
    mxArray *v;

    _MLVar *varPtr;

    int num_vars = num_ml_var(vars);

    const char **names = mxMalloc(num_vars * sizeof(char *));
    const char *structName;

    v = first_ml_var(vars);
    while (v) {
      name = mxGetName(v);
      v = next_ml_var(vars);
    }
	  
    count = 0;

    v = first_ml_var(vars);
    while (v) {
      name = mxGetName(v);
      
      for(i=0; i<count; i++) {
	if ( strcmp(names[i],name)==0 ) { 
	  Found = true;
	}
      }
      if ( !Found )
	names[count++] = mxGetName(v);

      v = next_ml_var(vars);
    }

    structName = mxMalloc(sizeof(char *));
    structName = lName;

    numUniqueVariables = num_vars / count;

    structArray = init_ml_vars();

    ret_array = mxCreateStructMatrix(numUniqueVariables, 1, count, &names[0]);
    mxSetName(ret_array, structName);

    /* Add exception handling for ret_array == NULL. */

    for(i=0; i<count; i++) {

      next = 0;
      for (j=i; j<num_vars; j+=count) {
	
	/* iterate over the vars list adding each mxArray */

	start = 0;

	v = first_ml_var(vars);
	while (v) {
	  if ( start == j ) {
	    mxSetFieldByNumber(ret_array, next, i, v);
	    next++;
	    break;
	  }
	  else {
	    v = next_ml_var(vars);
	    start++;
	  }
	} 
      }
    }      
    add_ml_var(structArray,ret_array);

    return structArray;

}

mxArray *
build_string_var(const char *name, int m, char **s)
{
    mxArray *array_ptr;

    array_ptr = mxCreateCharMatrixFromStrings(m, (const char **)s);    
    if (!array_ptr) {
	err_msg("Internal Error: Could not build string variable (%s:%d)",
		__FILE__, __LINE__);
	return FALSE;
    }
    mxSetName(array_ptr, name);

    return array_ptr;
}

/**
   Add into the Matlab workspace an array of strings named #name#.

   @param name Name of the variable. 
   @param m The number of strings in #s#.
   @param s Pointer to the array of string data.
   @param extend If TRUE extend the existing variable #name# 
*/

int
intern_strings(char *name, int m, char **s, int extend, mxArray **array_ptr)
{
  /*mxArray *array_ptr;*/
    int status;
    int exists;

    exists = is_existing_variable(name);
    if (!exists)
	add_new_variable(name);
    
#if 0
    /* Using the `extend' option and using return arguments is mutually
       exclusive. The return argument syntax overrides using the -k option. */
    if (num_return_args) {
	if (current_arg+1 > num_return_args)
	    return TRUE;	/* not an error technically */
	if (!(return_args[current_arg] =
	      mxCreateCharMatrixFromStrings(m, (const char **)s))) {
	    err_msg(
"Internal Error: Could not create variable for `%s' (%s:%d)\n",
		    name, __FILE__, __LINE__);
	    return FALSE;
	}
	mxSetName(return_args[current_arg], name);
	current_arg++;
	return TRUE;
    }
#endif
    if (extend && exists) {
	if (!extend_strings(name, 1, s)) {
	    err_msg(
"Internal Error: Could not create variable for `%s' (%s:%d)\n", 
		    name, __FILE__, __LINE__);
	    return FALSE;
	}
	else
	    return TRUE;
    }    

#if 0
    *array_ptr = build_string_var(name, m, (const char **)s);
#endif
    *array_ptr = build_string_var(name, m, (char **)s);

    /* *array_ptr = mxCreateCharMatrixFromStrings(m, (const char **)s);    */
    if (!*array_ptr) {
	err_msg(
"Internal Error: Could not intern strings (%s:%d)", __FILE__, __LINE__);
	return FALSE;
    }
    /*
    mxSetName(array_ptr, name);
    status = (mexPutArray(array_ptr, "caller") == 0);
    */
    return TRUE;
}

#ifdef TEST

/* 
   Test these functions. To test the return args feature, call with two
   variables. Maybe this should be five arguments?
*/

void
test_double(int extend)
{
    double data[2][3]={{7.0, 8.0, 9.0}, {10.0, 11.0, 12.0}};
    double data2[2][2]={{7.0, 8.0}, {9.0, 10.0}};
    double data3[2][2][3]={{{1, 2, 3},{4, 5, 6}}, {{7, 8, 9},{10, 11, 12}}};
    double vec[3]={2.0, 6.0, 9.0};
    double *data_ptr, *data2_ptr, *vec_ptr, *data3_ptr;
    double *start_ptr, *extend_ptr, *vstart_ptr, *vextend_ptr;
    int data_dim[2] = {2, 3};
    int data2_dim[2] = {2, 2};
    int data3_dim[3] = {2, 2, 3};
    int vec_dim[1] = {3};

    /* Load up the test data using memory allocated by mxMalloc(). */
    data_ptr = mxMalloc(6 * sizeof(double));
    memcpy(data_ptr, data, 6 * sizeof(double));

    data2_ptr = mxMalloc(4 * sizeof(double));
    memcpy(data2_ptr, data2, 4 * sizeof(double));

    data3_ptr = mxMalloc(12 * sizeof(double));
    memcpy(data3_ptr, data3, 12 * sizeof(double));

    vec_ptr = mxMalloc(3 * sizeof(double));
    memcpy(vec_ptr, vec, 3 * sizeof(double));

    /* Intern some variables. The state of extend makes no difference with
       these calls. */
    if (!extend) {
	intern("test1", 2, data_dim, data_ptr, FALSE);
	intern("test2", 2, data2_dim, data2_ptr, FALSE);
	intern("test3", 1, vec_dim, vec_ptr, FALSE);
	intern("test4", 3, data3_dim, data3_ptr, FALSE);
    }

    /* Load up new memory for extend tests. */
    start_ptr = mxMalloc(6 * sizeof(double));
    memcpy(start_ptr, data, 6 * sizeof(double));
    extend_ptr = mxMalloc(4 * sizeof(double));
    memcpy(extend_ptr, data2, 4 * sizeof(double));
    vstart_ptr = mxMalloc(3 * sizeof(double));
    memcpy(vstart_ptr, vec, 3 * sizeof(double));
    vextend_ptr = mxMalloc(3 * sizeof(double));
    memcpy(vextend_ptr, vec, 3 * sizeof(double));

    /* Intern with extend */
    if (extend) {
	intern("Mat", 2, data_dim, start_ptr, TRUE);
	intern("Mat", 2, data2_dim, extend_ptr, TRUE);

	intern("Mat2", 2, data_dim, start_ptr, TRUE);
	intern("Mat2", 2, data_dim, start_ptr, TRUE);

	intern("Vec", 1, vec_dim, vstart_ptr, TRUE);
	intern("Vec", 1, vec_dim, vextend_ptr, TRUE);
    }
}

void
test_strings(int extend)
{
    char *t1 = "test1";
    char *t2 = "This is a string with spaces";
    char *t3 = "Another string with spaces.";

    char *tm1 = mxMalloc(strlen(t1)+1);
    char *tm2 = mxMalloc(strlen(t2)+1);
    char *tsv[4];
    
    strcpy(tm1, t1);
    strcpy(tm2, t2);

    tsv[0] = mxMalloc(strlen(t2)+1);
    strcpy(tsv[0], t2);
    tsv[1] = mxMalloc(strlen(t3)+1);
    strcpy(tsv[1], t3);
    tsv[2] = mxMalloc(strlen(t2)+1);
    strcpy(tsv[2], t2);
    tsv[3] = mxMalloc(strlen(t3)+1);
    strcpy(tsv[3], t3);
    
    intern_strings("string1", 1, &tm1, extend);
    intern_strings("string2", 1, &tm2, extend);
    intern_strings("string3", 4, tsv, extend);
}

void 
mexFunction(int nlhs, mxArray *plhs[], const int nrhs, mxArray *prhs[])
{
    int string_tests = FALSE;
    int double_tests = FALSE;

    err_init(0);

    /* Set up the return args stuff. By default these are 0 and NULL. */
    if (nlhs > 0) {
	num_return_args = nlhs;
	return_args = plhs;
    }

    /* Look for command line arguments. */
    if (nrhs > 0 && mxIsChar(prhs[0])) {
	int buflen = mxGetN(prhs[0]) + 1;
	char *buf = mxMalloc(buflen);
	mxGetString(prhs[0], buf, buflen);
	
	if (strstr(buf, "-s"))
	    string_tests = TRUE;
	if (strstr(buf, "-d"))
	    double_tests = TRUE;
    }

    if (double_tests) {
	test_double(FALSE);
	test_double(TRUE);
    }
    
    if (string_tests) {
	test_strings(FALSE);
    }

    clear_existing_variables();	/* here temporarily */
}

#endif

/* 
 * $Log: extend.c,v $
 * Revision 1.3  2004/07/08 20:50:03  jimg
 * Merged release-3-4-5FCS
 *
 * Revision 1.27  2001/08/27 18:06:57  jimg
 * Merged release-3-2-5.
 * Revision 1.1.1.1.2.2  2004/04/16 19:34:05  dan
 * Fixed compile-time problems that gcc-2.95.3 was having that gcc-3.3
 * was not.
 *
 * Revision 1.2  2003/12/08 17:59:50  edavis
 * Merge release-3-4 into trunk
 *
 * Revision 1.1.1.1  2003/10/22 19:43:30  dan
 * Version of the Matlab CommandLine client which uses Matlab Structure
 * variables to maintain the shape of the underlying DODS data.
 *
 * Revision 1.30  2003/04/22 14:45:28  dan
 * Removed changes added to maintain DDS structure, these
 * changes have been placed on the structure-format1 branch
 * until the GUI is ready to use them.
 *
 * Revision 1.28  2003/01/29 15:43:52  dan
 * Resolved conflict on merge, caused by PWEST's removal
 * of the SLLIST includes in a previous update on the trunk.
 *
 *
 * Revision 1.26.2.2  2001/11/09 13:43:15  rmorris
 * clear_existing_variables() had a out-of-bounds indexing problem
 * into variables.name.  From 0 to n-1, not 1 to n.  Apparently went
 * unnoticed under UNIX but manifested itself under win32.
 *
 * Revision 1.26.2.1  2000/12/13 01:39:13  jimg
 * Moved a decrement operation from within a call to mxFree
 *
 * Revision 1.26  2000/11/22 23:43:00  jimg
 * Merge with pre-release 3.2
 *
 * Revision 1.16.18.13  2000/09/22 20:54:04  jimg
 * Added dmalloc stuff in DMALLOC guard
 *
 * Revision 1.25  2000/07/21 10:21:56  rmorris
 * Merged with win32-mlclient-branch.
 *
 * Revision 1.23.2.2  2000/07/10 19:39:32  rmorris
 * Attempts to minimize error messages during compilation on win32.
 *
 * Revision 1.23.2.1  2000/06/26 23:03:39  rmorris
 * Mods for port to win32.
 *
 * Revision 1.24  2000/06/20 21:40:09  jimg
 * Merged with 3.1.6
 *
 * Revision 1.16.18.12  2000/06/20 20:36:46  jimg
 * Imporved error messages and fixed some doc++ comments (although some
 * problems remain).
 *
 * Revision 1.16.18.11  2000/06/12 21:40:11  jimg
 * Moved the debugging code in extend_array so that it falls after the
 * declarations; gcc complained.
 *
 * Revision 1.16.18.10  2000/06/12 18:27:02  edavis
 * In extend_array(), allocate new_dims[] dynamically.
 *
 * Revision 1.16.18.9  2000/06/02 23:03:33  jimg
 * Fixes for AttrTables with no entries.
 * Added support for name translation and a DODS_ML_Real_Name attribute.
 * Changed the `size' attribtue's name to DODS_ML_Size.
 *
 * Revision 1.16.18.8  2000/06/01 01:33:53  jimg
 * Added support for writevals new attribute mode.
 *
 * Revision 1.16.18.7  2000/05/12 19:17:20  jimg
 * Made more functions static.
 * Added a get variable number function.
 *
 * Revision 1.16.18.6  2000/04/20 19:45:14  jimg
 * Changed some int variables to size_t.
 *
 * Revision 1.16.18.5  2000/04/19 05:39:06  jimg
 * Added conditional efence malloc defines.
 *
 * Revision 1.16.18.4  2000/04/11 21:48:59  jimg
 * Added support for n-dimensional arrays. the Concatenation option (see
 * loaddods.c and the -k switch) is *not* supported for N > 2, though.
 *
 * Revision 1.16.18.3  2000/04/01 00:36:06  jimg
 * Added error message when reading arrays with more than two dimensions.
 *
 * Revision 1.16.18.2  2000/04/01 00:11:30  jimg
 * Massive fixes for rm2cm and vice versa. Lots of changes in support of
 * n-dimensional code, but that is not complete yet.
 *
 * Revision 1.16.18.1  2000/02/19 00:36:14  jimg
 * Switched from the ML 4 to 5 API.
 *
 * Revision 1.16 1998/11/30 05:37:36 jimg Added the -T option to control
 * truncation of names longer than 19 characters
 *
 * Revision 1.15  1998/11/25 01:15:23  jimg
 * Identifiers limited to 19 chars. This is a temp fix; will update to
 * include ML 5 later 
 *
 * Revision 1.14 1998/11/11 18:49:53 jimg Changed the allocation of variables
 * in add_new_variable() from 5 to 500. Dan found that the loaddods/matlab
 * crash we see on Linux is caused by corruption of this data structure when
 * the data structure was initialized to 5 elements. Currently, Mathworks is
 * looking into the problem since it may be inside their code.
 *
 * Revision 1.13  1998/11/10 03:18:14  jimg
 * Added test stuff. This is fairly hokey...
 *
 * Revision 1.12  1998/07/13 20:31:10  jimg
 * Fixes from the final test of the new build process
 *
 * Revision 1.11  1998/06/09 03:59:32  jimg
 * Removed ugly debugging code.
 *
 * Revision 1.10.4.1  1998/05/15 17:43:52  jimg
 * Removed DODS_DEBUG define
 *
 * Revision 1.10  1998/03/23 22:54:39  jimg
 * Fixed extern_strings(...) so that it now works.
 *
 * Revision 1.9  1998/03/21 00:29:39  jimg
 * Added dummy function for extend_strings(). It prints an error message and
 * returns false.
 *
 * Revision 1.8  1998/03/21 00:19:51  jimg
 * Added some documentation.
 * Added intern_strings().
 *
 * Revision 1.7  1997/10/09 22:20:04  jimg
 * Resolved conflicts in merge of 2.14c to trunk.
 *
 * Revision 1.6  1997/10/04 00:33:47  jimg
 * Release 2.14c fixes
 *
 * Revision 1.4.6.1  1997/09/19 19:11:21  jimg
 * Moved config_writeval.h so that -V4_COMPAT flag works.
 *
 * Revision 1.5  1997/09/10 22:47:58  jimg
 * Added config_writeval.h to includes.
 *
 * Revision 1.4  1997/05/01 22:42:14  jimg
 * Added changes for the `return arguments' feature. See loaddods.c
 *
 * Revision 1.3  1997/03/27 22:55:30  jimg
 * Made rm2cm amd cm2rm globally visible.
 *
 * Revision 1.2  1997/03/07 04:54:09  jimg
 * Fixed comments.
 *
 * Revision 1.1  1997/03/07 04:50:26  jimg
 * Added. 
 */
