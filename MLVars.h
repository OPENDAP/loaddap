
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
extern void add_ml_var(MLVars *ml_vars, mxArray *array);
extern int num_ml_var(MLVars *ml_vars);
extern mxArray *first_ml_var(MLVars *ml_vars);
extern mxArray *next_ml_var(MLVars *ml_vars);
extern mxArray *current_ml_var(MLVars *ml_vars);

#endif /* _mlvars_h */
