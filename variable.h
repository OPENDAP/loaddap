
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

