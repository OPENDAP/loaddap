
/*
  C versions of the C++ defines used for debugging, etc.
*/

#ifndef __defines_h
#define __defines_h

#ifndef DBG
#ifdef DODS_DEBUG
#define DBG(x) x
#else
#define DBG(x)
#endif
#endif /* DBG */

#ifndef DBG2
#ifdef DODS_DEBUG2
#define DBG2(x) x
#else
#define DBG2(x)
#endif
#endif /* DBG2 */

#ifndef DBGM
#ifdef DODS_DEBUGM
#define DBGM(x) x
#else
#define DBGM(x)
#endif
#endif /* DBGM */

#ifndef TRUE
#define TRUE (1)
#define FALSE (0)
#endif /* TRUE */

#ifndef MAX
#define MAX(x,y) ((x > y) ? (x) : (y))
#endif /* MAX */

#ifdef NO_MX_FREE
#undef mxFree
#define mxFree(x) /* x */
#endif /* NO_MX_FREE */

#endif /* __defines_h */
