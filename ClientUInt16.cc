
// (c) COPYRIGHT URI/MIT 1999,2000
// Please read the full copyright statement in the file COPYRIGHT.
//
// Authors:
//      jhrg,jimg       James Gallagher (jgallagher@gso.uri.edu)

// Implementation for ClientInt16. See ClientByte.cc
//
// jhrg 11/1/96

<<<<<<< ClientUInt16.cc
#ifdef __GNUG__
#pragma implementation
#endif

#include "config_writedap.h"
=======
#include "config_writedap.h"
>>>>>>> 1.1.1.1.2.2

#include <assert.h>
#include <iostream>

#include <string>

#include "InternalErr.h"
#include "ClientUInt16.h"
#include "name_map.h"

extern name_map names;
extern bool ascii;
extern bool translate;
extern bool numeric_to_float;

UInt16 *
NewUInt16(const string &n)
{
    return new ClientUInt16(n);
}

ClientUInt16::ClientUInt16(const string &n) : UInt16(n)
{
    set_matlab_name(n);
}

BaseType *
ClientUInt16::ptr_duplicate()
{
    return new ClientUInt16(*this);
}

bool
ClientUInt16::read(const string &)
{
  throw InternalErr(__FILE__, __LINE__, "Called unimplemented read method");
}

void 
ClientUInt16::print_val(ostream &os, string, bool print_decl_p)
{
    if (print_decl_p)
        os << type_name() << endl << get_matlab_name() << endl;

    if (numeric_to_float) {
	dods_float64 df = _buf;
	if (ascii)
	    os << df << " ";
	else
	    os.write((char *)&df, sizeof(dods_float64));
    }
    else {
	if (ascii)
	    os << _buf << " ";
	else
	    os.write((char *)&_buf, sizeof(dods_uint16));
    }	
}


AttrTable &
ClientUInt16::getAttrTable()
{
    return _attr;
}

void 
ClientUInt16::setAttrTable(AttrTable &attr)
{
    _attr = attr;
}

void
ClientUInt16::set_name(const string &n)
{
    BaseType::set_name(n);
    set_matlab_name(n);
}

string 
ClientUInt16::get_matlab_name() const
{
    return _matlabName; 
}

void 
ClientUInt16::set_matlab_name(const string &name)
{ 
    _matlabName = names.lookup(name, translate);
}

// $Log: ClientUInt16.cc,v $
// Revision 1.3  2003/12/08 17:59:49  edavis
// Merge release-3-4 into trunk
//
// Revision 1.1.1.1  2003/10/22 19:43:20  dan
// Version of the Matlab CommandLine client which uses Matlab Structure
// variables to maintain the shape of the underlying DODS data.
// Revision 1.1.1.1.2.2  2003/10/29 19:03:21  dan
// Removed 'pragma interface' directive from all subclass
// source files.
//
// Revision 1.1.1.1.2.1  2003/10/27 16:41:30  dan
// Changed config include to 'config_writedap.h' to designate
// new version of 'writeval' now called 'writedap'.  The only
// substantive change in 'writedap' is that nested sequence
// variables are now supported.
//
// Revision 1.2  2003/10/23 18:34:02  dan
// Changed config include to config_writedap.h from config_writeval.h
// This is to remain consistent with the renaming used from loaddods
// to loaddap.  To support nested sequences writeval was modified
// to send an end-of-sequence marker to delimit sequence instances.
//
//
// Revision 1.8  2003/05/02 17:16:17  jimg
// I replaced the cast is ostream::write that was (void *) to (char *) to
// get this code to compile with gcc 3.2. This change is also needed for
// VC++, so I was able to remove some of the ifdef WIN32 lines. Also, in
// name_map I added a using namespace std; line.
//
// Revision 1.7  2001/08/27 18:06:57  jimg
// Merged release-3-2-5.
//
// Revision 1.6.2.1  2001/08/21 16:43:54  dan
// Added set/accessor methods for a new local member, matlab_name
// which stores the translated name of the variable so that it does
// not have to be translated during serialization.  This optimizes
// the amount of time spent in variable name translation to a single
// instance.
//
// Added a local method, set_name() which works with the BaseType
// pointer to update both the BaseType name member and local matlab_name
// member.
//
// Revision 1.6  2000/10/03 22:24:17  jimg
// Reorganized the CVS Log entries.
// Changed the read() mfunc definitions to match the 3.2 dap++ library.
//
// Revision 1.5  2000/08/30 00:13:29  jimg
// Merged with 3.1.7
//
// Revision 1.2.8.2  2000/08/25 23:37:36  jimg
// Added extern bool numeric_to_float global.
//
// Revision 1.4  2000/07/21 10:21:56  rmorris
// Merged with win32-mlclient-branch.
//
// Revision 1.3.2.2  2000/07/09 22:26:32  rmorris
// Mod's to increase portability, minimize ifdef's for win32.
//
// Revision 1.3.2.1  2000/06/26 23:00:12  rmorris
// Change for port to win32.
//
// Revision 1.3  2000/06/07 00:28:31  jimg
// Merged changes from version 3.1.4
//
// Revision 1.2.8.1  2000/06/02 23:03:32  jimg
// Fixes for AttrTables with no entries.
// Added support for name translation and a DODS_ML_Real_Name attribute.
// Changed the `size' attribtue's name to DODS_ML_Size.
//
// Revision 1.2  1999/04/30 17:06:55  jimg
// Merged with no-gnu and release-2-24
//
// Revision 1.1  1999/03/29 21:22:54  jimg
// Added
//
