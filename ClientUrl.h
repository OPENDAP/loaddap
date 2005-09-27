
// -*- C++ -*-

// (c) COPYRIGHT URI/MIT 1996,2000
// Please read the full copyright statement in the file COPYRIGHT.
//
// Authors:
//      jhrg,jimg       James Gallagher (jgallagher@gso.uri.edu)

// Interface for ClientUrl type. See ClientByte.h
//
// jhrg 1/12/95

#ifndef _clienturl_h
#define _clienturl_h 1

#include "Url.h"

class ClientUrl: public Url {
public:
    ClientUrl(const string &n = (char *)0);
    virtual ~ClientUrl() {}

    virtual BaseType *ptr_duplicate();
};

// $Log: ClientUrl.h,v $
// Revision 1.2  2003/12/08 17:59:49  edavis
// Merge release-3-4 into trunk
//
// Revision 1.1.1.1.2.1  2003/10/29 19:03:21  dan
// Removed 'pragma interface' directive from all subclass
// source files.
//
// Revision 1.1.1.1  2003/10/22 19:43:20  dan
// Version of the Matlab CommandLine client which uses Matlab Structure
// variables to maintain the shape of the underlying DODS data.
//
// Revision 1.4  2000/10/03 22:24:17  jimg
// Reorganized the CVS Log entries.
// Changed the read() mfunc definitions to match the 3.2 dap++ library.
//
// Revision 1.3  1999/04/30 17:06:56  jimg
// Merged with no-gnu and release-2-24
//
// Revision 1.2  1996/10/07 21:00:44  jimg
// Fixed Headers.
//
// Revision 1.1  1996/09/30 23:59:26  jimg
// Added.

#endif

