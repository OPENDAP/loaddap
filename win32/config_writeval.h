/* config_writeval.h.  Generated automatically by configure.  */
/* config.h.in.  Generated automatically from configure.in by autoheader.  */
#ifndef _DODS_CONFIG_WV_H_
#define _DODS_CONFIG_WV_H_

/* 
 * $Log: config_writeval.h,v $
 * Revision 1.1  2003/10/22 19:44:32  dan
 * Initial revision
 *
 * Revision 1.4  2003/01/29 19:30:20  dan
 * Merged with release-3-2-9
 *
 * Revision 1.3.2.1  2002/10/13 21:50:37  rmorris
 * Use unix text file format.  This is in support of a move toward
 * nightly builds for win32.
 *
 * Revision 1.3  2000/08/31 19:53:25  rmorris
 * Added not_used definition.
 *
 * Revision 1.2  2000/07/21 10:21:56  rmorris
 * Merged with win32-mlclient-branch.
 *
 * Revision 1.1.2.1  2000/06/26 22:58:49  rmorris
 * New fixed configuration file for win32.
 *
 * Revision 1.7  2000/04/01 01:16:23  jimg
 * Merged with release-3-1-2
 *
 * Revision 1.6.30.1  2000/02/19 00:36:14  jimg
 * Switched from the ML 4 to 5 API.
 *
 * Revision 1.6  1997/10/09 22:20:00  jimg
 * Resolved conflicts in merge of 2.14c to trunk.
 *
 * Revision 1.5  1997/10/04 00:33:43  jimg
 * Release 2.14c fixes
 *
 * Revision 1.4.6.1  1997/09/19 19:13:45  jimg
 * Added define of V4_COMPAT
 *
 * Revision 1.4  1997/06/06 04:56:49  jimg
 * *** empty log message ***
 *
 * Revision 1.3  1997/05/06 00:35:08  jimg
 * Protect BIG/LITTLE ENDIAN define
 *
 * Revision 1.2  1997/02/06 19:40:02  jimg
 * Added DODS_ROOT and {BIG,LITTLE}_ENDIAN.
 *
 * Revision 1.1  1996/10/25 23:21:27  jimg
 * Added.
 * */

#if defined(HAVE_CONFIG_H) || defined(_WIN32)

/* *** CHANGE THIS CONSTANT IN config_writeval.h.in !!! */
#define MAX_QUEUE_LEN 8192

/* Define this if building the server with Matlab 5 */
#define V4_COMPAT 1

/* One of these must be defined! Enclose in #ifndef ... #endif since
   some machines contain this define (in netinet/in.h). */
#ifndef BIG_ENDIAN
/* #undef BIG_ENDIAN */
#endif
#ifndef LITTLE_ENDIAN
/* #undef LITTLE_ENDIAN */
#endif

#ifndef DODS_ROOT
#ifdef WIN32
#define DODS_ROOT "C:\\DODS"
#else
#define DODS_ROOT "/usr/local/DODS"
#endif
#endif

#endif /* HAVE_CONFIG_H */

#define not_used

#endif /* _DODS_CONFIG_H_ */


