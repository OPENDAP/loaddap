
// (c) COPYRIGHT URI/MIT 1996,2000
// Please read the full copyright statement in the file COPYRIGHT.
//
// Authors:
//      jhrg,jimg       James Gallagher (jgallagher@gso.uri.edu)

// Implementation for ClientInt32. See ClientByte.cc
//
// jhrg 11/1/96

#ifdef __GNUG__
#pragma implementation
#endif

#include "config_writedap.h"

#include <assert.h>
#include <iostream>

#include <Pix.h>
#include <string>

#include "InternalErr.h"
#include "ClientUInt32.h"
#include "name_map.h"

extern name_map names;
extern bool ascii;
extern bool translate;
extern bool numeric_to_float;

UInt32 *
NewUInt32(const string &n)
{
    return new ClientUInt32(n);
}

ClientUInt32::ClientUInt32(const string &n) : UInt32(n)
{
    set_matlab_name(n);
}

BaseType *
ClientUInt32::ptr_duplicate()
{
    return new ClientUInt32(*this);
}

bool
ClientUInt32::read(const string &)
{
  throw InternalErr(__FILE__, __LINE__, "Called unimplemented read method");
}

void 
ClientUInt32::print_val(ostream &os, string, bool print_decl_p)
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
	    os.write((char *)&_buf, sizeof(dods_uint32));
    }	
}


AttrTable &
ClientUInt32::getAttrTable()
{
    return _attr;
}

void 
ClientUInt32::setAttrTable(AttrTable &attr)
{
    _attr = attr;
}

void
ClientUInt32::set_name(const string &n)
{
    BaseType::set_name(n);
    set_matlab_name(n);
}

string 
ClientUInt32::get_matlab_name() const
{
    return _matlabName; 
}

void 
ClientUInt32::set_matlab_name(const string &name)
{ 
    _matlabName = names.lookup(name, translate);
}

// $Log: ClientUInt32.cc,v $
// Revision 1.2  2003/10/23 18:34:02  dan
// Changed config include to config_writedap.h from config_writeval.h
// This is to remain consistent with the renaming used from loaddods
// to loaddap.  To support nested sequences writeval was modified
// to send an end-of-sequence marker to delimit sequence instances.
//
// Revision 1.1.1.1  2003/10/22 19:43:20  dan
// Version of the Matlab CommandLine client which uses Matlab Structure
// variables to maintain the shape of the underlying DODS data.
//
// Revision 1.20  2003/05/02 17:16:17  jimg
// I replaced the cast is ostream::write that was (void *) to (char *) to
// get this code to compile with gcc 3.2. This change is also needed for
// VC++, so I was able to remove some of the ifdef WIN32 lines. Also, in
// name_map I added a using namespace std; line.
//
// Revision 1.19  2003/01/29 15:43:52  dan
// Resolved conflict on merge, caused by PWEST's removal
// of the SLLIST includes in a previous update on the trunk.
//
// Revision 1.18  2001/08/27 18:06:57  jimg
// Merged release-3-2-5.
//
// Revision 1.17.2.2  2002/09/05 22:26:49  pwest
// Removed includes to SLList and DLList. These are not necessary and no longer
// supported.
//
// Revision 1.17.2.1  2001/08/21 16:44:19  dan
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
// Revision 1.17  2000/10/03 22:24:17  jimg
// Reorganized the CVS Log entries.
// Changed the read() mfunc definitions to match the 3.2 dap++ library.
//
// Revision 1.16  2000/08/30 00:13:29  jimg
// Merged with 3.1.7
//
// Revision 1.13.12.2  2000/08/25 23:37:15  jimg
// Added extern bool numeric_to_float global.
//
// Revision 1.15  2000/07/21 10:21:56  rmorris
// Merged with win32-mlclient-branch.
//
// Revision 1.14.2.2  2000/07/09 22:26:33  rmorris
// Mod's to increase portability, minimize ifdef's for win32.
//
// Revision 1.14.2.1  2000/06/26 23:00:12  rmorris
// Change for port to win32.
//
// Revision 1.14  2000/06/07 00:28:31  jimg
// Merged changes from version 3.1.4
//
// Revision 1.13.12.1  2000/06/02 23:03:32  jimg
// Fixes for AttrTables with no entries.
// Added support for name translation and a DODS_ML_Real_Name attribute.
// Changed the `size' attribtue's name to DODS_ML_Size.
//
// Revision 1.13  1999/03/24 06:23:43  brent
// convert String.h to std lib <string>, convert to packages regex -- B^2
//
// Revision 1.12  1998/11/20 08:41:22  jimg
// Fixed the fix for the extra newline separators; it was broken for arrays
//
// Revision 1.11  1998/11/19 20:53:33  jimg
// Fixed the extra newline bug.
//
// Revision 1.10  1998/09/16 23:02:23  jimg
// Removed write_val() and replaced it with print_val() and print_all_vals()
//
// Revision 1.9  1998/08/03 16:39:52  jimg
// Fixed write_val mfunc so that outermost is no longer used
//
// Revision 1.8  1997/10/04 00:40:09  jimg
// Added changes to support the new deserialize() mfunc and write_val().
//
// Revision 1.7  1997/06/06 04:02:29  jimg
// Added ASCII output option to writeval. Uses a global `flag'.
//
// Revision 1.6  1997/04/19 00:55:00  jimg
// Changed ouput stream to simplify reading (for loaddods).
//
// Revision 1.5  1997/02/06 20:43:42  jimg
// Fixed log messages
//
// Revision 1.4  1997/01/10 06:49:39  jimg
// Changed call to name_map::lookup() so that non-alphanumerics are mapped to
// underscore. 
//
// Revision 1.3  1997/01/08 21:33:51  jimg
// Changed `dods_int32' to `dods_uint32' in ClientUInt32::write_val().
//
// Revision 1.2  1996/11/23 05:12:16  jimg
// Added support fr variable renaming via the name_map object.
//
// Revision 1.1  1996/11/02 00:03:25  jimg
// Added.
//
