
// (c) COPYRIGHT URI/MIT 1996,2000
// Please read the full copyright statement in the file COPYRIGHT.
//
// Authors:
//      jhrg,jimg       James Gallagher (jgallagher@gso.uri.edu)

// Implementation for the class ClientStructure. See ClientByte.cc
//
// jhrg 1/12/95

#ifdef _GNUG_
#pragma implementation
#endif

#include "config_writedap.h"

#include <assert.h>
#include <iostream>
#include <string>

#include <Pix.h>

#include "InternalErr.h"
#include "ClientStructure.h"
#include "ClientSequence.h"
#include "ClientArray.h"

#include "name_map.h"

extern name_map names;
extern bool translate;

extern void smart_newline(ostream &os, Type type);

Structure *
NewStructure(const string &n)
{
    return new ClientStructure(n);
}

BaseType *
ClientStructure::ptr_duplicate()
{
    return new ClientStructure(*this);
}

ClientStructure::ClientStructure(const string &n) : Structure(n)
{
    set_matlab_name(n);
}

ClientStructure::~ClientStructure()
{
}

// For this `Client' class, run the read mfunc for each of variables which
// comprise the structure. 

bool
ClientStructure::read(const string &)
{
  throw InternalErr(__FILE__, __LINE__, "Called unimplemented read method");
}

void 
ClientStructure::print_val(ostream &os, string space, bool print_decls)
{
    for (Pix p = first_var(); p; next_var(p)) {
	var(p)->print_val(os, "", print_decls);
	smart_newline(os,var(p)->type());
    }
}

AttrTable &
ClientStructure::getAttrTable()
{
    return _attr;
}

void 
ClientStructure::setAttrTable(AttrTable &attr)
{
    _attr = attr;
}

void
ClientStructure::set_name(const string &n)
{
    BaseType::set_name(n);
    set_matlab_name(n);
}

string 
ClientStructure::get_matlab_name() const
{
    return _matlabName; 
}

void 
ClientStructure::set_matlab_name(const string &name)
{ 
    _matlabName = names.lookup(name, translate);
}

// $Log: ClientStructure.cc,v $
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
// Revision 1.22  2003/04/22 14:42:55  dan
// Removed changes added to maintain DDS structure, these
// changes have been placed on the structure-format1 branch
// until the GUI is ready to use them.
//
// Revision 1.20  2003/01/29 15:43:52  dan
// Resolved conflict on merge, caused by PWEST's removal
// of the SLLIST includes in a previous update on the trunk.
//
// Revision 1.19  2001/08/27 18:06:57  jimg
// Merged release-3-2-5.
//
// Revision 1.18.2.4  2002/09/05 22:26:49  pwest
// Removed includes to SLList and DLList. These are not necessary and no longer
// supported.
//
// Revision 1.18.2.3  2002/01/23 03:54:18  dan
// Added a call to smart_newline subsequent to each var(p)->print_val
// call in the ClientStructure::print_val method.
//
// Revision 1.18.2.2  2002/01/16 00:32:32  jimg
// A while ago I changed the way Sequences are deserialized in the DAP. To
// accommodate these changes the code in ClientSequence, ClientStructure and
// process_data had to be fixed.
// NB: It would be better to not use BaseType::print_val(...) but instead
// use a mixin for writing out writeval's stuff. See the asciival code for
// an example.
//
// Revision 1.18.2.1  2001/08/21 16:49:47  dan
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
// Revision 1.18  2000/10/03 22:24:17  jimg
// Reorganized the CVS Log entries.
// Changed the read() mfunc definitions to match the 3.2 dap++ library.
//
// Revision 1.17  2000/07/21 10:21:56  rmorris
// Merged with win32-mlclient-branch.
//
// Revision 1.16.2.2  2000/07/09 22:26:32  rmorris
// Mod's to increase portability, minimize ifdef's for win32.
//
// Revision 1.16.2.1  2000/06/26 23:00:12  rmorris
// Change for port to win32.
//
// Revision 1.16  2000/06/07 00:28:31  jimg
// Merged changes from version 3.1.4
//
// Revision 1.15.12.1  2000/06/02 23:03:32  jimg
// Fixes for AttrTables with no entries.
// Added support for name translation and a DODS_ML_Real_Name attribute.
// Changed the `size' attribtue's name to DODS_ML_Size.
//
// Revision 1.15  1999/03/24 06:23:43  brent
// convert String.h to std lib <string>, convert to packages regex -- B^2
//
// Revision 1.14  1998/11/20 08:41:22  jimg
// Fixed the fix for the extra newline separators; it was broken for arrays
//
// Revision 1.13  1998/11/19 20:53:33  jimg
// Fixed the extra newline bug.
//
// Revision 1.12  1998/11/05 21:40:58  jimg
// Fixed a bug where print_val() on structure variables was not called with
// print_decl_p passed. Instead the member function was called with the flag
// forced to false. This made it impossible for loaddods to read the the data
// stream.
//
// Revision 1.11  1998/09/16 23:02:22  jimg
// Removed write_val() and replaced it with print_val() and print_all_vals()
//
// Revision 1.10  1998/08/03 16:39:51  jimg
// Fixed write_val mfunc so that outermost is no longer used
//
// Revision 1.9  1997/10/04 00:40:08  jimg
// Added changes to support the new deserialize() mfunc and write_val().
//
// Revision 1.8  1997/04/19 00:54:59  jimg
// Changed ouput stream to simplify reading (for loaddods).
//
// Revision 1.7  1997/02/06 20:43:41  jimg
// Fixed log messages
//
// Revision 1.6  1997/01/10 06:49:38  jimg
// Changed call to name_map::lookup() so that non-alphanumerics are mapped to
// underscore.
//
// Revision 1.5  1996/11/23 05:12:14  jimg
// Added support for variable renaming via the name_map object.
//
// Revision 1.4  1996/10/25 23:13:59  jimg
// Changed/Fixed write_val() so that it uses smart_newline().
//
// Revision 1.3  1996/10/23 23:46:45  jimg
// Modified so that writeval outputs a `recursive' data stream. Thus any
// data type can be written and later read without being flattened. This
// change was made so that the NSCAT data which is represented as a Sequence
// of Structures.
//
// Revision 1.2  1996/10/07 21:00:30  jimg
// Fixed Headers.
//
// Revision 1.1  1996/09/30 23:59:21  jimg
// Added.
