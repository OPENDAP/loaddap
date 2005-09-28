
// (c) COPYRIGHT URI/MIT 1999
// Please read the full copyright statement in the file COPYRIGHT.
//
// Authors:
//      jhrg,jimg       James Gallagher (jgallagher@gso.uri.edu)

// Implementation for ClientInt16. See ClientByte.cc
//
// jhrg 1/12/95

#include "config_writedap.h"

#include <iostream>
#include <string>

#include "InternalErr.h"
#include "ClientInt16.h"
#include "name_map.h"

extern name_map names;
extern bool translate;
extern bool numeric_to_float;
extern bool ascii;

Int16 *
NewInt16(const string &n)
{
    return new ClientInt16(n);
}

ClientInt16::ClientInt16(const string &n) : Int16(n)
{
    set_matlab_name(n);
}

BaseType *
ClientInt16::ptr_duplicate()
{
    return new ClientInt16(*this);
}

bool
ClientInt16::read(const string &)
{
  throw InternalErr(__FILE__, __LINE__, "Called unimplemented read method");
}

void 
ClientInt16::print_val(ostream &os, string, bool print_decl_p)
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
	    os.write((char *)&_buf, sizeof(dods_int16));
    }	
}

AttrTable &
ClientInt16::getAttrTable()
{
    return _attr;
}

void 
ClientInt16::setAttrTable(AttrTable &attr)
{
    _attr = attr;
}

void
ClientInt16::set_name(const string &n)
{
    BaseType::set_name(n);
    set_matlab_name(n);
}

string 
ClientInt16::get_matlab_name() const
{
    return _matlabName; 
}

void 
ClientInt16::set_matlab_name(const string &name)
{ 
    _matlabName = names.lookup(name, translate);
}

// $Log: ClientInt16.cc,v $
// Revision 1.4  2004/07/08 20:50:03  jimg
// Merged release-3-4-5FCS
//
// Revision 1.3  2003/12/08 17:59:49  edavis
// Merge release-3-4 into trunk
//
// Revision 1.1.1.1  2003/10/22 19:43:18  dan
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
// Revision 1.9  2003/05/02 17:16:17  jimg
// I replaced the cast is ostream::write that was (void *) to (char *) to
// get this code to compile with gcc 3.2. This change is also needed for
// VC++, so I was able to remove some of the ifdef WIN32 lines. Also, in
// name_map I added a using namespace std; line.
//
// Revision 1.8  2003/01/29 15:43:52  dan
// Resolved conflict on merge, caused by PWEST's removal
// of the SLLIST includes in a previous update on the trunk.
//
// Revision 1.7  2001/08/27 18:06:57  jimg
// Merged release-3-2-5.
//
// Revision 1.6.2.2  2002/09/05 22:26:49  pwest
// Removed includes to SLList and DLList. These are not necessary and no longer
// supported.
//
// Revision 1.6.2.1  2001/08/21 16:43:28  dan
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
// Revision 1.2.8.2  2000/08/25 23:38:06  jimg
// Added extern bool numeric_to_float global.
//
// Revision 1.4  2000/07/21 10:21:56  rmorris
// Merged with win32-mlclient-branch.
//
// Revision 1.3.2.2  2000/07/09 22:26:32  rmorris
// Mod's to increase portability, minimize ifdef's for win32.
//
// Revision 1.3.2.1  2000/06/26 22:54:26  rmorris
// Mods for port to win32.
//
// Revision 1.3  2000/06/07 00:28:31  jimg
// Merged changes from version 3.1.4
//
// Revision 1.2.8.1  2000/06/02 23:03:31  jimg
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
