
// (c) COPYRIGHT URI/MIT 1996
// Please read the full copyright statement in the file COPYRIGH.  
//
// Authors:
//      jhrg,jimg       James Gallagher (jgallagher@gso.uri.edu)

// Implementation for ClientUrl. See ClientByte.cc
//
// jhrg 1/12/95

// $Log: ClientUrl.cc,v $
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
// Revision 1.5  1999/03/24 06:23:43  brent
// convert String.h to std lib <string>, convert to packages regex -- B^2
//
// Revision 1.4  1996/10/23 23:46:46  jimg
// Modified so that writeval outputs a `recursive' data stream. Thus any
// data type can be written and later read without being flattened. This
// change was made so that the NSCAT data which is represented as a Sequence
// of Structures.
//
// Revision 1.3  1996/10/09 15:26:58  jimg
// Removed redundant read and writeval mfuncs.
//
// Revision 1.2  1996/10/07 21:00:31  jimg
// Fixed Headers.
//
// Revision 1.1  1996/09/30 23:59:25  jimg
// Added.

#include <string>

#include "ClientUrl.h"

ClientUrl *
NewUrl(const string &n)
{
    return new ClientUrl(n);
}

ClientUrl::ClientUrl(const string &n) : Url(n)
{
}

BaseType *
ClientUrl::ptr_duplicate()
{
    return new ClientUrl(*this);
}
