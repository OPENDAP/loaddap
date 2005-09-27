
#ifdef __cplusplus
extern "C" {
#endif

void err_msg(const char *m, ...) ;
void perr_msg(const char *m, ...) ;
void msg(const char *m, ...);
int err_init(int compat_mode);

#ifdef __cplusplus
}
#endif
