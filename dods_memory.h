
#ifndef _DODS_MEMORY_H
#define	_DODS_MEMORY_H

#ifdef	__cplusplus
extern "C" {
#endif

extern void *dods_calloc(size_t, size_t);
extern void dods_free(void *);
extern void *dods_malloc(size_t);
extern void *dods_realloc(void *, size_t);

#ifdef	__cplusplus
}
#endif

#endif	/* _DODS_MEMORY_H */
