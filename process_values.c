/* 
   (c) COPYRIGHT URI/MIT 2000
   Please read the full copyright statement in the file COPYRIGHT.

   Authors:
      grf		Glenn Fleirl (glenn@lake.mit.edu)
      jhrg,jimg		James Gallagher (jgallagher@gso.uri.edu)
*/

/** These functions are used to read from the data stream written by the
    writeval program.

    8/17/2000 jhrg
*/

#include "config_writedap.h"

static char id[] not_used ={"$Id: process_values.c,v 1.3 2003/12/08 17:59:50 edavis Exp $"};

#include <errno.h>

#define MATLAB_MEX_FILE 
#include <mex.h>

#include "variable.h"
#include "MLVars.h"
#include "extend.h"

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

#include "defines.h"

#ifdef WIN32
#define isnan(y) _isnan(y)
#endif

#define MAX_STR 256
#define BIG_STR 8192

/** Does the user what the names of variables returned to result argument
    zero? */
extern int output_variable_names;

/** If NUM_RETURN_ARGS is false, use the default mechanism where variables are
   directly interned in Matlab. If true, return values via plhs[]. If true
   the value of NUM_RETURN_ARGS is the number of values to return. After
   reading this many variables, discard any remaining ones. 
   @see loaddods.c */
extern int num_return_args;

/** Global array of arrays used to hold NUM_RETURN_ARGS. 
    @see loaddods.c */
extern mxArray **return_args;

/** When using the return arguments feature of loaddods, this holds the
    current argument number. This is re-initialized by loaddods.cc:init()
    every time the loaddods command is run. 

    @see num_return_args
    @see return_args 
    @see loaddods.c */
extern int current_arg;

/** Does the user want verbose outout messages. On by default. */
extern int verbose;

/** Does the user want variables with the same names which are loadded two or
    more times in the same call to loaddods (you can pass more than one URL
    to loaddods) to be concatenated? */
extern int extend_existing_variables;

/** This flag indicates that the user supplied the fullnames switch.
    Fullnames means that structure fields are interned by prefixing the
    field name with the name of the structure. This option is a hold over
    from the ML 4 days before structures (which still aren't fully supported
    8/17/2000 jhrg) and short (19 character) name limits (i.e., you probably
    did not want this on all the time.
    @see loaddods.c */
extern int fullnames;

/** This flag indicates whether loaddods should use ML's structure variables
    when interning values. A true values mean to use Structures, false means
    to dump variables into the workspace (this was previously the only
    option). The default value is false. 

    Initially this will only be true when we're handling the -A option for
    attributes. */
static int use_structures = 0;

static int do_error(FILE *, variable **vectors, char *, int, MLVars *);
static int do_float(FILE *, variable **vectors, char *, int, MLVars *);
static int do_string(FILE *, variable **vectors, char *, int, MLVars *);
static int do_list(FILE *, variable **vectors, char *, int, MLVars *);
static int do_array(FILE *, variable **vectors, char *, int, MLVars *);
static int do_structure(FILE *, variable **vectors, char *, int, MLVars *);
static int do_sequence(FILE *, variable **vectors, char *, int, MLVars *);
static int do_grid(FILE *, variable **vectors, char *, int, MLVars *);
static int do_attributes(FILE *, variable **vectors, char *, int, MLVars *);

typedef (*reader_type)(FILE *, variable **vectors, char *, int, MLVars *);

/* NB: The Byte and Int32 types are mapped to Float64 since Matlab 4
   represents numbers using only 64 bit floating point values. */

struct typeitem {
    char typename[64];
    reader_type do_type;
} typelist[] = {
    {"Error", do_error},
    {"Byte", do_float},
    {"Int16", do_float},
    {"UInt16", do_float},
    {"Int32", do_float},
    {"UInt32", do_float},
    {"Float32", do_float},
    {"Float64", do_float},
    {"String", do_string},
    {"Url", do_string},
    {"List", do_list},
    {"Array", do_array},
    {"Structure", do_structure},
    {"Sequence", do_sequence},
    {"Function", do_sequence},
    {"Grid", do_grid},
    {"Attributes", do_attributes},
    {"", NULL}
};

/** Build a vector containing the names of all the variables (as they appear
    in the datasets) for each variable read from the DODS server. */

static void
build_variable_name_vector()
{
    char *name;
    char **names, **names_tmp;
    size_t number_of_vars = (size_t)get_number_of_variables();

    names = (char **)mxCalloc(number_of_vars, sizeof(char *));
    DBGM(mexPrintf("mxCalloc (%s:%d): %x\n", __FILE__, __LINE__, 
		   names));

    /* Load strings into names vector. */ 
    name = get_variable_name(TRUE);
    names_tmp = names;
    while (name) {
	*names++ = name;
	name = get_variable_name(FALSE);
    }

    names = names_tmp;

    return_args[0] = mxCreateCharMatrixFromStrings(number_of_vars, 
						   (const char **)names);    
}

/** Strip out certain characters from a string.

    @param str Operate on this char *.
    @param c Look for this character and remove it. */
static void
strip(char *dest, char *src, char c)
{
    while (*src) {
	if (*src == c)
	    *dest = *src++;
	else
	    *dest++ = *src++;
    }
    *dest = '\0';
}
/**
  Given a type name, return a pointer to the reader function (which is
  named do_* in this source file). Returns NULL if there is no matching
  function for TYPENAME. */
static reader_type
get_reader(char *typename)
{
    size_t k = 0;
    do {
	if (strcmp(typelist[k].typename, typename) == 0)
	    return typelist[k].do_type;
    } while (typelist[++k].typename[0]);

    return NULL;
}

/**
  Read a line from writeval which should have only one word on it. Store the
  word in NAME. 

  NB: if this function is called to read in the name of a component in an
  aggregate type *and* we are prefixing names using the aggregate names then
  NAME must contain that prefix string. If *not* be *sure* to set the first
  element of NAME to '\0' before calling this function. */
static int
read_name(FILE *fin, char name[MAX_STR], int verbose)
{
    size_t items;
    char line[256];
    DBG2(msg("At the top of the while loop in read_name\n"));
    /* Pick up the next line that does not start with a newline character. */
    while (!feof(fin)) {
	DBG2(msg("Inside the while loop in read_name\n"));
	if (!fgets(line, 255, fin)) {
	    if (errno == EINTR)
		continue;
	    if (verbose)
		err_msg(\
"Error: Could not read variable name or datatype. This may be an error\n\
internal to loaddods or it may be caused by an earlier error.\n");
	    return FALSE;
	}
	if (line[0] != '\n')
	    break;
    }

    DBG(msg("read_name: line: `%s'\n", line));

    items = sscanf(line, "%s", &name[strlen(name)]);
    if (items != 1) {
	if (verbose)
	    err_msg(\
"Error:Could not parse variable name or datatype. This may be an error\n\
internal to loaddods or it may be caused by an earlier error.\n");
	return FALSE;
    }

    return TRUE;
}

/* 
   Note: when reading an array description, `length' is the number of
   dimensions in the array.

   NB: See note above about NAME.
*/

static int
read_vec_descript(FILE *fin, char element_type[MAX_STR], char name[MAX_STR], 
		  int *length)
{
    size_t items;
    char line[256];

    if (feof(fin)) {
	DBG(msg("EOF found.\n"));
	return FALSE;
    }

    fgets(line, 255, fin);

    DBG(msg("line: `%s'\n", line));

    items = sscanf(line, "%s %s %d", &element_type[0], 
		   &name[strlen(name)], length);
    if (items !=3) {
	err_msg(\
"Error: Could not read type, name and/or length of vector. This may be an\n\
error internal to loaddods or it may be caused by an earlier error.\n");
	return FALSE;
    }
    
    return TRUE;
}

/*
  Read the dimension info from the stream and load it into an array.
*/
#define MAX_LN_LEN 256

static int
read_array_dims(FILE *fin, int ndims, int *dims)
{
    int i;			/* dimension currently being read */
    size_t length;
    char line[MAX_LN_LEN], *line_p;
    char dimension[MAX_LN_LEN];
    
    if (feof(fin)) {
	DBG(msg("EOF found.\n"));
	return FALSE;
    }

    fgets(line, MAX_LN_LEN-1, fin);
    line_p = &line[0];		/* line_p is used iterate over elements */

    DBG(msg("line: `%s'\n", line_p));

    /* Read in the dimension sizes and compute total number of elements.
       NB: Look for dimensions with size equal to one. These are dimensions
       that have been `nulled' by a constraint expression (reducing the
       dimensionality of an array from, say, three to two. Make sure that the
       first two elements of DIMS contain the valid sizes and not the size of
       the `nulled' dimension (which is one by definition). */
    length = strcspn(line_p, " \n");
    mxAssertS(length < MAX_LN_LEN, "");

    strncpy(dimension, line_p, length);
    dimension[length] = '\0';
    i = 0;
    while (length) {
	int dim = atoi(dimension);
	    
	if (i > ndims) {
	    err_msg(\
"Internal Error: Confusion reading array dimensions (%s:%d)", __FILE__,
		    __LINE__);
	    return FALSE;
	}
	dims[i++] = dim;

	line_p += length + 1;
	mxAssertS(line_p - line < MAX_LN_LEN-1, "");

	length = strcspn(line_p, " \n");
	strncpy(dimension, line_p, length);
	mxAssertS(line_p - line + length < MAX_LN_LEN, "");

	dimension[length] = '\0';
    }

    return TRUE;
}

/*
  Read a name and length from writeval.

  NB: See note above about NAME.
*/

static int
read_aggregate(FILE *fin, char name[MAX_STR], int *length)
{
    size_t items;
    char line[256];
    fgets(line, 255, fin);

    DBG(msg("line: `%s'\n", line));

    items = sscanf(line, "%s %d", &name[strlen(name)], length);
    if (items != 2) {
	err_msg(\
"Error: Could not read name and/or length of aggregate. This may be an error\n\
internal to loaddods or it may be caused by an earlier error.\n");
	return FALSE;
    }
    
    return TRUE;
}

/*
  Read a line and discard it. This function is intended to be used to read
  the newline that follows a block of data. 

  NB: I used fseek() for this but that caused failures on SunOS (and maybe
  Solaris?) for transfers that were an integral power of 2 (e.g., 32^2,
  512^2, 1023^2).
*/

static int
read_trailing_newline(FILE *fin)
{
    char dummy[256];
    if (feof(fin)) {
	err_msg("Internal Error: Unexpected EOF. (%s:%d)\n", __FILE__, 
		__LINE__);
	return FALSE;
    }

    return fgets(dummy, 255, fin) != 0;
}

/**
   This function returns false, indicating that an error was found either in
   writeval or by the server. Instead of returning data, writeval has
   returned an error. */

static int 
do_error(FILE *fin, variable **vectors, char *prefix, int outermost, 
	 MLVars *vars)
{
    char value[BIG_STR];
    size_t i = 0;

    DBG(msg("In do_error\n"));
    value[0]='\0';
    fgets(value, BIG_STR-1, fin);
    do {
	i += strlen(&value[i]);
	fgets(&value[i], BIG_STR-(i+1), fin);
    } while (!feof(fin) && i < BIG_STR && value[i] != '\0');
	
    DBG(msg("Error message from writeval before processing:\n%s\n", value));

    strip(value, value, '"');		/* Strip out double quotes. */
    
    err_msg("Error: %s\n", value);

    return 0;
}

/** Read a single float value from the input stream. */
static int
do_float(FILE *fin, variable **vectors, char *prefix, int outermost,
	 MLVars *vars)
{
    double y;
    char name[MAX_STR] = "";
    variable *var;

    DBG2(msg("do_float: prefix: %s\n", prefix));

    /* If prefix is non-null load it into NAME. */
    if (fullnames && prefix[0]) {
	strcpy(name, prefix);
	strcat(name, "_");
    }
    else {
	name[0]='\0';
    }

    DBG2(msg("do_float: prefix: %s, name: %s\n", prefix, name));

    if (!read_name(fin, name, TRUE))
	return FALSE;

    DBG2(msg("do_float: name: %s\n", name));

    if (use_structures) {
	mxArray *variable;
	fread(&y, sizeof(double), 1, fin);
	if (isnan(y)) {
	    DBG(msg("Found NaN!\n"));
	    y = mxGetNaN();
	}

	variable = build_var(name, 0, 0, &y);
	add_ml_var(vars, variable);

    }
    else {
	/* Check to see if this vector already exists. If not, create it,
	   reassign the global list of vectors and set the local pointer VAR
	   to the newly added element. */
	var = find_var(*vectors, name);
	if (!var)
	    var = *vectors = add_var(*vectors, name, float64);

	/* Add the value. */
	fread(&y, sizeof(double), 1, fin);
	if (isnan(y)) {
	    DBG(msg("Found NaN!\n"));
	    y = mxGetNaN();
	}
	add_float64_value(var, y);
    
	DBG(msg("Float value read: %lf\n", y));
    }

#ifdef DODS_DEBUG2
    {
	union {
	    unsigned int ul[2];
	    double d;
	} v;
	v.d = y;
	msg("y (hex): %x %x\n", v.ul[0], v.ul[1]);
    }
#endif

    DBG2(msg("do_float: read: %s = %f\n", name, y));

    return TRUE;
}

/* This macro must take into account that a quoted string might have a
   newline as its first character. In that case the length will be two and
   *((buf) + (len) - 2) will be true for the first quote character (but it is
   being used to test for the closing quote)! For the first line of a quoted
   string, return true for IS_END_QUOTE only if the length of the string is
   not 2. For subsequent lines the test is OK. 11/20/2000 jhrg */
#define IS_END_QUOTE(buf, len, first_line) \
((first_line) ? (*((buf) + (len) - 2) == '"' && (len != 2)) \
             : (*((buf) + (len) - 2) == '"'))

#define IS_END_NEWLINE(buf, len) (*((buf) + (len) - 1) == '\n')

/** Read a string value from the #fin# stream. If the string starts with a
   double quote, ignore newlines until the matching end quote is found. If
   the string does not start with a double quote, then read to the end of the
   line. In either case, discard the last newline character. 

   NB: If the string is quoted the end quote will appear two chars back
   from the last char read. This is because writeval always appends a newline
   to every string. 

   @return The string in newly allocated storage. The caller must free this
   memory. */

static char *
read_string_value(FILE *fin)
{
    int len;
    int is_quoted = FALSE;
    int first_line = TRUE;
    char *value = NULL;
    char *lbuf = mxMalloc(sizeof(char) * MAX_STR);

    fgets(lbuf, MAX_STR-1, fin);
    is_quoted = (*lbuf == '"');

    len = strlen(lbuf);
    value = mxMalloc(sizeof(char) * (len + 1));
    strncpy(value, lbuf, len);
    *(value + len) = '\0';

    /* The test for !IS_END_NEWLINE covers strings larger than MAX_STR.
       The `first_line' parameter in IS_END_QUOTE is part of a fix for
       quoted strings which span several lines *and* whose first character
       after the opening quote is a \n. See the comment above. 11/20/2000
       jhrg */
    while ((is_quoted) ? !IS_END_QUOTE(lbuf, len, first_line)
	   : !IS_END_NEWLINE(lbuf, len))
      {
	first_line = FALSE;
	fgets(lbuf, MAX_STR-1, fin);
	len = strlen(lbuf);
	value = mxRealloc(value, sizeof(char) * (len + strlen(value) + 1));
	strncat(value, lbuf, len);
	strcat(value, "");
      }

    mxFree(lbuf);

    /* Now strip off the trailing newline */
    len = strlen(value);
    *(value + len -1) = '\0';
    
#if 0
    /* Remove double quotes for ML. */
    strip(value, value, '"');
#endif
    return value;
}

static int 
do_string(FILE *fin, variable **vectors, char *prefix, int outermost,
	  MLVars *vars)
{
    variable *var;
    char *value;
    char name[MAX_STR] = "";

    /* If prefix is non-null load it into NAME. */
    if (fullnames && prefix[0]) {
	strcpy(name, prefix);
	strcat(name, "_");
    }
    else {
	name[0]='\0';
    }

    if (!read_name(fin, name, TRUE))
	return FALSE;

    if (use_structures) {
	mxArray *variable;
	value = read_string_value(fin);
	variable = build_string_var(name, 1, (char **)&value);
	add_ml_var(vars, variable);
	mxFree(value);
    }
    else {
	/* Check to see if this vector already exists. If not, create it,
	   reassign the global list of vectors and set the local pointer VAR
	   to the newly added element. */
	var = find_var(*vectors, name);
	if (!var)
	    var = *vectors = add_var(*vectors, name, string);

	/* Read and add the value; also read the trailing newline. */
	value = read_string_value(fin);
	add_string_value(var, value);
	mxFree(value);
    }

    DBG(msg("do_string: read: %s = `%s'\n", name, value));

    return TRUE;
}

void
print_array_creation_msg(char *name, int num_elems, int ndims, int dims[])
{
  switch (ndims)
    {
    case 0:
      msg("Creating scalar %s.\n", name);
      break;
    case 1:
      msg("Creating vector %s with %d elements.\n",
	  name, num_elems);
      break;
    default: 
      {
	int j;
	msg("Creating matrix %s (", name);
	for (j = 0; j < ndims-1; j++)
	  msg("%d x ", dims[j]);
	msg("%d) with %d elements.\n", dims[ndims-1], num_elems);
	break;
      }
    }
}

static int
do_array(FILE *fin, variable **vectors, char *prefix, int outermost, 
	 MLVars *vars)
{
  size_t i;
  int ndims;
  int *dims;
  size_t num_elems;
  char element_type[MAX_STR];
  char name[MAX_STR] = "";	/* PREFIX + short_name */

  /* If prefix is non-null load it into NAME. */
  if (fullnames && prefix[0]) {
    strcpy(name, prefix);
    strcat(name, "_");
  }
  else {
    name[0]='\0';
  }

  if (!read_vec_descript(fin, element_type, name, &ndims))
    return FALSE;

  dims = mxCalloc((size_t)ndims, sizeof(int));
  DBGM(mexPrintf("mxCalloc (%s:%d): %x\n", __FILE__, __LINE__, dims));

  if (!read_array_dims(fin, ndims, dims))
    return FALSE;

  /* ndims = remove_null_axis(ndims, dims); */
  num_elems = (size_t)count_elements(ndims, dims);

  /* Process arrays of simple numeric types specially. */
  if (strcmp(element_type, "Byte") == 0
      || strcmp(element_type, "Int16") == 0
      || strcmp(element_type, "UInt16") == 0
      || strcmp(element_type, "Int32") == 0
      || strcmp(element_type, "UInt32") == 0
      || strcmp(element_type, "Float32") == 0
      || strcmp(element_type, "Float64") == 0) 
    {
      double *yp = (double *) mxCalloc(num_elems, sizeof(double));
      DBGM(mexPrintf("mxCalloc (%s:%d): %x\n", __FILE__, __LINE__, 
		     yp));

      fread(yp, sizeof(double), num_elems, fin);

      if (use_structures)
	{
	  mxArray *variable = build_var(name, ndims, dims, yp);
	  if (!variable)
	    return FALSE;
	  add_ml_var(vars, variable);
	}
      else
	{
	  if (!intern(name, ndims, dims, yp, extend_existing_variables))
	    return FALSE;
	}

      if (verbose && !num_return_args)
	print_array_creation_msg(name, num_elems, ndims, dims);
    }
  else if (strcmp(element_type, "String") == 0)
    {
      if (use_structures)
	{
	  int i;
	  mxArray *variable;
	  char **value;
	  value = (char **)mxMalloc(num_elems * sizeof(char *));

	  for (i = 0; i < num_elems; ++i)
	    value[i] = read_string_value(fin);

	  variable = build_string_var(name, num_elems, value);
	  add_ml_var(vars, variable);

	  for (i = 0; i < num_elems; ++i)
	    mxFree(value[i]);
	  mxFree(value);
	}
      else 
	{
	  char *value;
	  variable *var;
	  int i;
	  /* Check to see if this vector already exists. If not, create it,
	     reassign the global list of vectors and set the local pointer VAR
	     to the newly added element. */
	  var = find_var(*vectors, name);
	  if (!var)
	    var = *vectors = add_var(*vectors, name, string);

	  /* Read and add the value; also read the trailing newline. */
	  for (i = 0; i < num_elems; ++i)
	    {
	      value = read_string_value(fin);
	      add_string_value(var, value);
	    }

	  mxFree(value);
	}
    }
  else 
    {			/* Do this for non-numeric arrays. */
      int status;
      reader_type exectype = get_reader(element_type);

      if (!exectype) {
	err_msg("Internal Error: Unknown datatype. (%s:%d)\n", __FILE__,
		__LINE__);
	return FALSE;
      }

      DBG2(msg("do_array: Processing: %s\n", element_type)); 

      for (i = 0; i < num_elems; ++i)
	{
	  status = (*exectype)(fin, vectors, name, outermost, vars);
	  if (!status)
	    return FALSE;
	}
    }

  if (!read_trailing_newline(fin))
    return FALSE;

  return TRUE;
}

/*
  Read the List variable into a single column vector.

  Untested.
*/

static int
do_list(FILE *fin, variable **vectors, char *prefix, int outermost,
	MLVars *vars)
{
    int length;
    int dims[1];
    char element_type[MAX_STR];
    char name[MAX_STR] = "";	/* PREFIX + short_name */

    /* If prefix is non-null load it into NAME. */
    if (fullnames && prefix[0]) {
	strcpy(name, prefix);
	strcat(name, "_");
    }
    else {
	name[0]='\0';
    }

    if (!read_vec_descript(fin, element_type, name, &length))
	return FALSE;

    /* Process arrays of simple numeric types specially. */
    if (strcmp(element_type, "Byte") == 0
	|| strcmp(element_type, "Int32") == 0
	|| strcmp(element_type, "Float64") == 0) {
	double *yp = (double *) mxCalloc((size_t) length, sizeof(double));
	DBGM(mexPrintf("mxCalloc (%s:%d): %x\n", __FILE__, __LINE__, 
		     yp));

	/* Note that according to a message from ML support, I should free yp
	   after once I've interned the data in a ML variable (in the
	   caller's workspace. But this always leads to messages from assert
	   that memory is corrupted. 2/21/2000 jhrg */

	if (fread(yp, sizeof(double), (size_t) length, fin) != length) {
	    err_msg(\
"Error: Could not read all the list values. This may be an error\n\
internal to loaddods or it may be caused by an earlier error.\n");
	    return FALSE;
	}

	dims[0] = length;
	if (use_structures) {
	    mxArray *variable = build_var(name, 1, dims, yp);
	    if (!variable)
		return FALSE;
	    add_ml_var(vars, variable);
	}
	else {
	    if (!intern(name, 1, dims, yp, extend_existing_variables))
		return FALSE;
	}

	if (verbose && !num_return_args)
	    msg("Creating vector %s with %d elements.\n", name, length);
    }
    else {
	int i;
	int status;
	reader_type exectype = get_reader(element_type);

	if (!exectype) {
	    err_msg("Internal Error: Unknown datatype. (%s:%d)\n", __FILE__, 
		    __LINE__);
	    return FALSE;
	}

	DBG2(msg("do_list: Processing: %s\n", element_type)); 

	for (i = 0; i < length; ++i) {
	    status = (*exectype)(fin, vectors, name, outermost, vars);
	    if (!status) {
		return FALSE;
	    }
	}
    }

    if (!read_trailing_newline(fin))
	return FALSE;

    return TRUE;
}

static int
do_structure(FILE *fin, variable **vectors, char *prefix, int outermost,
	     MLVars *vars)
{
    int num_elems;
    int status;
    size_t i;
    char name[MAX_STR] = "";	/* PREFIX + short_name */
    MLVars *my_vars;		/* Holds ML struct */

    if (use_structures)
	my_vars = init_ml_vars();
    else
	my_vars = NULL;

    /* If prefix is non-null load it into NAME. */
    if (fullnames && prefix[0]) {
	strcpy(name, prefix);
	strcat(name, "_");
    }
    else {
	name[0]='\0';
    }

    if (!read_aggregate(fin, name, &num_elems))
	return FALSE;

    DBG2(msg("do_struct: name: %s, elems: %d\n", name, num_elems));

    for (i = 0; i < num_elems; ++i) {
	char  element_type[MAX_STR];
	reader_type exectype;

	element_type[0]='\0';
	if (!read_name(fin, element_type, TRUE))
	    return FALSE;

	DBG2(msg("do_struct: Processing: %s\n", element_type)); 

	exectype = get_reader(element_type);

	if (!exectype) {
	    err_msg("Internal Error: Unknown datatype. (%s:%d)\n", __FILE__,
		    __LINE__);
	    return FALSE;
	}

	status = (*exectype)(fin, vectors, name, outermost, my_vars);
	if (!status) {
	    return FALSE;
	}
    }
    
    if (use_structures) {
	mxArray *my_struct = build_ml_vars(name, my_vars);

	status = 1;
	if (vars)
	    add_ml_var(vars, my_struct);
	else
	    status = (mexPutArray(my_struct, "caller") == 0);
    }

    if (!status) 
	err_msg("Internal Error: Could not intern variable %s. (%s:%d)\n", 
		name, __FILE__, __LINE__);

    if (my_vars)
	clear_ml_vars(my_vars);

    return status;
}

/** @param vars Ignored. */
static int
do_sequence(FILE *fin, variable **vectors, char *prefix, int outermost,
	    MLVars *vars)
{
    int num_elems;
    size_t i;
    char name[MAX_STR] = "";	/* PREFIX + short_name */

    /* If prefix is non-null load it into NAME. For Seqs, don't do this on
       the outermost level to conserve name length. */
    if (fullnames && prefix[0] && !outermost) {
	strcpy(name, prefix);
	strcat(name, "_");
    }
    else {
	name[0]='\0';
    }

    if (!read_aggregate(fin, name, &num_elems))
	return FALSE;

    DBG(msg("do_seq: name: %s, elems: %d\n", name, num_elems));

    /* NB: Run the loop util all the data is used up when reading the
       outermost sequence, otherwise read the elements only once. */
    do {
	for (i = 0; i < num_elems; ++i) {
	    int status;
	    char  element_type[MAX_STR];
	    reader_type exectype;

	    element_type[0]='\0';
	    DBG(msg("element type: %s\n", element_type));
	    if (!read_name(fin, &element_type[0], FALSE)) {
		if (outermost)
		    return TRUE;
		else {
		    return FALSE;
		}
	    }

	    DBG(msg("do_seq: Processing: %s\n", element_type)); 

	    exectype = get_reader(element_type);
	    if (!exectype) {
		err_msg("Internal Error: Unknown datatype (%s) (%s:%d)\n", 
			element_type, __FILE__, __LINE__);
		return FALSE;
	    }

	    status = (*exectype)(fin, vectors, name, FALSE, vars);
	    if (!status) {
		return FALSE;
	    }
	}
    } while (outermost && !feof(fin) && !ferror(fin));

    return TRUE;
}

/** @param vars Ignored. */
static int
do_grid(FILE *fin, variable **vectors, char *prefix, int outermost, 
	MLVars *vars)
{
    char name[MAX_STR] = "";	/* PREFIX + short_name */
    int num_elems;

    /* If prefix is non-null load it into NAME. */
    if (fullnames && prefix[0]) {
	strcpy(name, prefix);
	strcat(name, "_");
    }
    else {
	name[0]='\0';
    }

    if (!read_name(fin, name, TRUE))
	return FALSE;

    DBG(msg("do_grid: name: %s\n", name));

    {
	size_t i;
	int status;
	char part[MAX_STR];
	for (i = 0; i < 2; ++i) {
	    size_t j;

	    part[0]='\0';
	    if (!read_aggregate(fin, part, &num_elems))
		return FALSE;

	    DBG(msg("do_grid: part: %s, num_elems: %d\n", part, num_elems));

	    for (j = 0; j < num_elems; ++j) {
		char dummy[MAX_STR];

		DBG2(msg("do_grid: Processing: Array\n")); 

		dummy[0]='\0';
		if (!read_name(fin, dummy, TRUE))
		    return FALSE;

		status = do_array(fin, vectors, name, FALSE, vars);
		if (!status) {
		    return FALSE;
		}
	    }
	}
    }

    return TRUE;
}

/* This function is a little different than all the other do_* functions
   since it *only* handles the attributes while the other handle both
   attributes and variables.

   @param fin Input stream.
   @param vectors Ignored.
   @param prefix Ignored.
   @param outermost Ignored.
   @param vars Ignored. 
*/
static int
do_attributes(FILE *fin, variable **vectors, char *prefix, int outermost,
	      MLVars *vars)
{
    int num_elems;
    size_t i;
    int tmp_verbose;		/* holds the value of the verbose flag */
    int tmp_use_structs;
    char name[MAX_STR] = "";	/* PREFIX + short_name */
    MLVars *my_vars = init_ml_vars();

    name[0]='\0';

    /* Accessing attribute information requires that the caller use the
       return argument feature of loaddods. There must be exactly one return
       argument. If that's not the case, exit now before we start reading
       stuff. */
    if (num_return_args != 1 || current_arg != 0) {
	err_msg(\
"Error: loaddods must be called with a return argument when reading\n\
       attributes. Type `help loaddods' for more information.\n");
	return FALSE;
    }

    /* Now kill the verbose mode since it generates bogus messages about
       creating vectors when that's not really true, at least not in the
       sense that vector variables are being added to the user's workspace. */
    tmp_verbose = verbose;	/* save the current value */
    verbose = 0;

    /* Record the current value of the global switch use_structures and set
       the switch to TRUE. Undo this before exiting. */
    tmp_use_structs = use_structures;
    use_structures = 1;

    if (!read_aggregate(fin, name, &num_elems))
	return FALSE;

    DBG2(msg("do_attributes: name: %s, elems: %d\n", name, num_elems));

    for (i = 0; i < num_elems; ++i) {
	int status;
	char  element_type[MAX_STR];
	reader_type exectype;

	element_type[0]='\0';
	if (!read_name(fin, element_type, TRUE))
	    return FALSE;

	DBG2(msg("do_attributes: Processing: %s\n", element_type)); 

	exectype = get_reader(element_type);

	if (!exectype) {
	    err_msg("Internal Error: Unknown datatype. (%s:%d)\n", __FILE__,
		    __LINE__);
	    return FALSE;
	}

	status = (*exectype)(fin, vectors, name, outermost, my_vars);
	if (!status) {
	    return FALSE;
	}
    }

    /* Now reset verbose mode (since the value will carry over from one run
       of loaddods to the next, we must reset the value from above. */
    verbose = tmp_verbose;

    /* Reset the use_structures global switch. */
    use_structures = tmp_use_structs;

    /* Return the attribute structure as the single return value of loaddods.
       Currently, this is the only way to get attributes back into Matlab. */
    if (num_return_args == 1&& current_arg == 0)
	return_args[current_arg++] = build_ml_vars("", my_vars);
    else
	err_msg("Internal Error: Expected one return argument. (%s:%d)\n", 
		__FILE__, __LINE__);

    if (my_vars)
	clear_ml_vars(my_vars);

    return TRUE;
}

int
process_values(FILE *fin)
{
  variable *vectors = NULL;
  char datatype[MAX_STR];
  
    /* Note that read_name() is called with VERBOSE == FALSE since this
       call will fail when there are no more variables to read. */
    datatype[0]='\0';
    while (read_name(fin, &datatype[0], FALSE) && !ferror(fin) && !feof(fin)) {
	int status;		/* Return value from exectype(). */
	reader_type exectype;

	DBG2(msg("Processing: %s\n", datatype));

	exectype = get_reader(datatype);
	if (!exectype) {
	    err_msg("Internal Error: Unknown datatype (%s). (%s:%d)\n", 
		    datatype, __FILE__, __LINE__);
	    return FALSE;
	}

	/* The initial value of the MLVars list is NULL. This will change
	   when full support for ML structures is added. */
	status = (*exectype)(fin, &vectors, "", TRUE, NULL);
	if (!status) {
	  return FALSE;
	}

#ifndef WIN32
	fflush(stderr);
	fflush(stdout);
#endif
	datatype[0]='\0';
    }

    if (!transfer_builtup_arrays(vectors))
      return FALSE;

    if (output_variable_names)
      build_variable_name_vector();

    if (vectors)
	free_vars(vectors);

    return TRUE;
}

/** Intern the vectors built up from ctor variables multiple calls to
    do_float and do_string. Make each vector a column vector. */

int
transfer_builtup_arrays(variable *var)
{
  while (var) {
    int status;
    switch (var->type) {
    case float64:
      /* Note that in the following call, ndims == 1. By sending
	 the address of var->length we fool intern into thinking it
	 is getting a one element array which holds the size of the
	 single dimension of var->data. */

      status = intern(var->name, 1, &var->length, var->data,
		      extend_existing_variables);
      if (status && verbose && !num_return_args)
	msg("Creating vector `%s' with %d elements.\n",
	    var->name, var->length);
      break;

    case string: {
      status = intern_strings(var->name, var->length, var->sdata, 
			      extend_existing_variables);
      if (status && verbose && !num_return_args)
	msg("Creating string vector `%s' with %d elements.\n", 
	    var->name, var->length);
      break;
    }

    default:
      status = FALSE;
      break;
    }

    if (!status) {
      err_msg("Internal Error: Could not add vector %s. (%s:%d)\n",
	      var->name, __FILE__, __LINE__);
      return FALSE;
    }

    var = var->next;
  }

  return TRUE;
}

/* 
   $Log: process_values.c,v $
   Revision 1.3  2003/12/08 17:59:50  edavis
   Merge release-3-4 into trunk


   Revision 1.10  2003/04/22 14:46:47  dan
   Removed changes added to maintain DDS structure, these
   changes have been placed on the structure-format1 branch
   until the GUI is ready to use them.

   Revision 1.8  2003/01/29 15:43:52  dan
   Resolved conflict on merge, caused by PWEST's removal
   of the SLLIST includes in a previous update on the trunk.

   Revision 1.7  2001/10/14 01:12:27  jimg
   Merged with release-3-2-7.

   Revision 1.4.2.4  2002/07/31 03:05:51  dan
   Removed strip of quotation character in read_string_value
   function.

   Revision 1.4.2.3  2001/10/10 21:14:37  jimg
   Changed include of mex.h from "mex.h" to <mex.h> so that it won't be
   included in the Makefile.in's dependencies. This maens builders are
   less likely to have to run make depend.

   Revision 1.6  2001/09/29 00:08:01  jimg
   Merged with 3.2.6.

   Revision 1.4.2.2  2001/08/29 23:11:35  edavis
   Changed C++ comment to a C comment.

   Revision 1.4.2.1  2001/08/21 16:51:22  dan
   Removed call to remove null axis from singleton matrices.

   Revision 1.4  2000/11/22 23:43:00  jimg
   Merge with pre-release 3.2

   Revision 1.3.4.1  2000/11/22 23:38:42  jimg
   Fixed the treatment of quoted strings. If a qutoed string starts with a
   newline (as its first character) then do_string() was interpreting the
   starting quote as the end quote.

   Revision 1.1.2.5  2000/09/22 20:54:04  jimg
   Added dmalloc stuff in DMALLOC guard

   Revision 1.3  2000/08/31 22:33:14  rmorris
   Small change to us _isnan() instead of isnan() under win32.

   Revision 1.2  2000/08/30 00:13:29  jimg
   Merged with 3.1.7

   Revision 1.1.2.4  2000/08/21 19:51:42  jimg
   Fixed processing of arrays of strings, although in a particularly gross
   way. Maybe this code can be cleaned up so that its easier to work with...

   Revision 1.1.2.3  2000/08/21 18:38:58  jimg
   Moved the addition of structure and array handlers to a spearate branch
   (more-datatype-handlers).

   Revision 1.1.2.1  2000/08/17 23:56:48  jimg
   Factored the code that reads and processes variables and values into
   process_values.c (from loaddods.c).
*/
