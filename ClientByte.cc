
// (c) COPYRIGHT URI/MIT 1996,2000
// Please read the full copyright statement in the file COPYRIGHT.
//
// Authors:
//      jhrg,jimg       James Gallagher (jgallagher@gso.uri.edu)

// Implementation for ClientByte. See the comments in ClientByte.h
//
// jhrg 9/24/96

#ifdef __GNUG__
#pragma implementation
#endif

// The NewByte `helper function' creates a pointer to the a ClientByte and
// returns that pointer. It takes the same arguments as the class's ctor. If
// any of the variable classes are subclassed (e.g., to make a new Byte like
// HDFByte) then the corresponding function here, and in the other class
// definition files, needs to be changed so that it creates an instnace of
// the new (sub)class. Continuing the earlier example, that would mean that
// NewByte() would return a HDFByte, not a Byte.
//
// It is important that these function's names and return types do not change
// - they are called by the parser code (for the dds, at least) so if their
// names changes, that will break.
//
// The declarations for these fuctions (in util.h) should *not* need
// changing. 

#include "config_writedap.h"

#include <stdio.h>
#include <assert.h>

#include <iostream>
#include <Pix.h>
#include <string>

#include "InternalErr.h"
#include "ClientByte.h"
#include "name_map.h"

extern name_map names;
extern bool translate;
extern bool numeric_to_float;
extern bool ascii;

Byte *
NewByte(const string &n)
{
    return new ClientByte(n);
}

ClientByte::ClientByte(const string &n) : Byte(n)
{
    set_matlab_name(n);
}

BaseType *
ClientByte::ptr_duplicate()
{
    return new ClientByte(*this);
}

bool
ClientByte::read(const string &)
{
  throw InternalErr(__FILE__, __LINE__, "Called unimplemented read method");
}

void
ClientByte::print_val(ostream &os, string, bool print_decl_p)
{
    if (print_decl_p)
      os << type_name() << endl << get_matlab_name() << endl;
      //os << type_name() << endl << names.lookup(name(), translate) << endl;

    if (numeric_to_float) {
	dods_float64 df = _buf;
	if (ascii)
	    os << df << " ";
	else
	    os.write((char *)&df, sizeof(dods_float64));
    }
    else {
	if (ascii)
	    os << (unsigned int)_buf << " ";
	else
	    os.write((char *)&_buf, sizeof(dods_byte));
    }	
}

AttrTable &
ClientByte::getAttrTable()
{
    return _attr;
}

void 
ClientByte::setAttrTable(AttrTable &attr)
{
    _attr = attr;
}

void
ClientByte::set_name(const string &n)
{
    BaseType::set_name(n);
    set_matlab_name(n);
}

string 
ClientByte::get_matlab_name() const
{
    return _matlabName; 
}

void 
ClientByte::set_matlab_name(const string &name)
{ 
    _matlabName = names.lookup(name, translate);
}

// $Log: ClientByte.cc,v $
// Revision 1.2  2003/10/23 18:34:02  dan
// Changed config include to config_writedap.h from config_writeval.h
// This is to remain consistent with the renaming used from loaddods
// to loaddap.  To support nested sequences writeval was modified
// to send an end-of-sequence marker to delimit sequence instances.
//
// Revision 1.1.1.1  2003/10/22 19:43:17  dan
// Version of the Matlab CommandLine client which uses Matlab Structure
// variables to maintain the shape of the underlying DODS data.
//
// Revision 1.22  2003/05/02 17:16:17  jimg
// I replaced the cast is ostream::write that was (void *) to (char *) to
// get this code to compile with gcc 3.2. This change is also needed for
// VC++, so I was able to remove some of the ifdef WIN32 lines. Also, in
// name_map I added a using namespace std; line.
//
// Revision 1.21  2003/01/29 15:43:52  dan
// Resolved conflict on merge, caused by PWEST's removal
// of the SLLIST includes in a previous update on the trunk.
//
// Revision 1.20  2001/08/27 18:06:57  jimg
// Merged release-3-2-5.
//
// Revision 1.19.2.2  2002/09/05 22:26:49  pwest
// Removed includes to SLList and DLList. These are not necessary and no longer
// supported.
//
// Revision 1.19.2.1  2001/08/21 16:42:24  dan
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
// Revision 1.19  2000/10/03 22:24:17  jimg
// Reorganized the CVS Log entries.
// Changed the read() mfunc definitions to match the 3.2 dap++ library.
//
// Revision 1.18  2000/08/30 00:13:29  jimg
// Merged with 3.1.7
//
// Revision 1.15.12.2  2000/08/25 23:38:17  jimg
// Added extern bool numeric_to_float global.
//
// Revision 1.17  2000/07/21 10:21:55  rmorris
// Merged with win32-mlclient-branch.
//
// Revision 1.16.2.2  2000/07/09 22:26:32  rmorris
// Mod's to increase portability, minimize ifdef's for win32.
//
// Revision 1.16.2.1  2000/06/26 22:51:36  rmorris
// Mods for port to win32.
//
// Revision 1.16  2000/06/07 00:28:31  jimg
// Merged changes from version 3.1.4
//
// Revision 1.15.12.1  2000/06/02 23:03:31  jimg
// Fixes for AttrTables with no entries.
// Added support for name translation and a DODS_ML_Real_Name attribute.
// Changed the `size' attribtue's name to DODS_ML_Size.
//
// Revision 1.15  1999/03/24 06:23:42  brent
// convert String.h to std lib <string>, convert to packages regex -- B^2
//
// Revision 1.14  1998/11/20 08:41:21  jimg
// Fixed the fix for the extra newline separators; it was broken for arrays
//
// Revision 1.13  1998/11/19 20:53:33  jimg
// Fixed the extra newline bug.
//
// Revision 1.12  1998/09/16 23:02:18  jimg
// Removed write_val() and replaced it with print_val() and print_all_vals()
//
// Revision 1.11  1998/08/03 16:39:44  jimg
// Fixed write_val mfunc so that outermost is no longer used
//
// Revision 1.10  1997/10/04 00:39:59  jimg
// Added changes to support the new deserialize() mfunc and write_val().
//
// Revision 1.9  1997/06/23 17:53:16  jimg
// When writing the ascii output for Byte, cast _buf to unsigned int because
// unsigned char prints as binary. Is this a C++ `feature' or a GNU bug?
//
// Revision 1.8  1997/06/06 04:01:02  jimg
// Added ASCII output option to writeval. Uses a global `flag'.
//
// Revision 1.7  1997/04/19 00:54:49  jimg
// Changed ouput stream to simplify reading (for loaddods).
//
// Revision 1.6  1997/02/06 20:43:32  jimg
// Fixed log messages
//
// Revision 1.5  1997/01/10 06:49:27  jimg
// Changed call to name_map::lookup() so that non-alphanumerics are mapped to
// underscore.
//
// Revision 1.4  1996/11/23 05:12:03  jimg
// Added support for variable renaming via the name_map object.
//
// Revision 1.3  1996/10/23 23:46:36  jimg
// Modified so that writeval outputs a `recursive' data stream. Thus any
// data type can be written and later read without being flattened. This
// change was made so that the NSCAT data which is represented as a Sequence
// of Structures.
//
// Revision 1.2  1996/10/07 21:00:22  jimg
// Fixed Headers.
//
// Revision 1.1  1996/09/30 23:58:53  jimg
// Added.
