
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

    extern mxArray *build_ml_vars(const char *name, MLVars *vars);

#ifdef __cplusplus
}
#endif
