
// (c) COPYRIGHT URI/MIT 1996,2000
// Please read the full copyright statement in the file COPYRIGHT.
//
// Authors:
//      jhrg,jimg       James Gallagher (jgallagher@gso.uri.edu)

// implementation for ClientGrid. See ClientByte.
//
// jhrg 10/4/96

#ifdef __GNUG__
#pragma implementation
#endif

#include "config_writedap.h"

#include <assert.h>
#include <iostream>
#include <Pix.h>
#include <string>

#include "InternalErr.h"
#include "ClientGrid.h"
#include "name_map.h"

extern name_map names;
extern bool translate;

Grid *
NewGrid(const string &n)
{
    return new ClientGrid(n);
}

BaseType *
ClientGrid::ptr_duplicate()
{
    return new ClientGrid(*this);
}

ClientGrid::ClientGrid(const string &n) : Grid(n)
{
}

ClientGrid::~ClientGrid()
{
}

bool
ClientGrid::read(const string &)
{
  throw InternalErr(__FILE__, __LINE__, "Called unimplemented read method");
}

void
ClientGrid::print_val(ostream &os, string space, bool print_decl_p)
{
    if (print_decl_p) {
	os << type_name() << endl << names.lookup(name(), translate) << endl;

	// We can only store one array in a grid right now.
	os << "array 1" << endl;
    }

    array_var()->print_val(os, space, print_decl_p);
    os << endl;

    if (print_decl_p) {
	os << "maps ";
	int i = 0;
	for (Pix p = first_map_var(); p; next_map_var(p))
	    ++i;
	os << i << endl;
    }

    for (Pix p = first_map_var(); p; next_map_var(p)) {
	map_var(p)->print_val(os, space, print_decl_p);
	os << endl;
    }
}

AttrTable &
ClientGrid::getAttrTable()
{
    return _attr;
}

void 
ClientGrid::setAttrTable(AttrTable &attr)
{
    _attr = attr;
}

// $Log: ClientGrid.cc,v $
// Revision 1.2  2003/10/23 18:34:02  dan
// Changed config include to config_writedap.h from config_writeval.h
// This is to remain consistent with the renaming used from loaddods
// to loaddap.  To support nested sequences writeval was modified
// to send an end-of-sequence marker to delimit sequence instances.
//
// Revision 1.1.1.1  2003/10/22 19:43:18  dan
// Version of the Matlab CommandLine client which uses Matlab Structure
// variables to maintain the shape of the underlying DODS data.
//
// Revision 1.17  2003/01/29 15:45:05  dan
// Resolved conflict on merge, caused by PWEST's removal
// of the SLLIST includes in a previous update on the trunk.
//
// Revision 1.16.2.1  2002/09/05 22:26:49  pwest
// Removed includes to SLList and DLList. These are not necessary and no longer
// supported.
//
// Revision 1.16  2000/10/03 22:24:17  jimg
// Reorganized the CVS Log entries.
// Changed the read() mfunc definitions to match the 3.2 dap++ library.
//
// Revision 1.15  2000/07/21 10:21:56  rmorris
// Merged with win32-mlclient-branch.
//
// Revision 1.14.2.2  2000/07/09 22:26:32  rmorris
// Mod's to increase portability, minimize ifdef's for win32.
//
// Revision 1.14.2.1  2000/06/26 22:52:33  rmorris
// Mods for port to win32.
//
// Revision 1.14  2000/06/07 00:28:31  jimg
// Merged changes from version 3.1.4
//
// Revision 1.13.12.1  2000/06/02 23:03:31  jimg
// Fixes for AttrTables with no entries.
// Added support for name translation and a DODS_ML_Real_Name attribute.
// Changed the `size' attribtue's name to DODS_ML_Size.
//
// Revision 1.13  1999/03/24 06:23:42  brent
// convert String.h to std lib <string>, convert to packages regex -- B^2
//
// Revision 1.12  1998/11/20 08:41:22  jimg
// Fixed the fix for the extra newline separators; it was broken for arrays
//
// Revision 1.11  1998/09/16 23:02:20  jimg
// Removed write_val() and replaced it with print_val() and print_all_vals()
//
// Revision 1.10  1998/08/03 16:39:47  jimg
// Fixed write_val mfunc so that outermost is no longer used
//
// Revision 1.9  1997/10/04 00:40:02  jimg
// Added changes to support the new deserialize() mfunc and write_val().
//
// Revision 1.8  1997/04/19 00:54:52  jimg
// Changed ouput stream to simplify reading (for loaddods).
//
// Revision 1.7  1997/02/06 20:43:36  jimg
// Fixed log messages
//
// Revision 1.6  1997/01/10 06:49:31  jimg
// Changed call to name_map::lookup() so that non-alphanumerics are mapped to
// underscore. 
//
// Revision 1.5  1996/11/23 05:12:07  jimg
// Added support for variable renaming via the name_map object.
//
// Revision 1.4  1996/10/25 23:07:30  jimg
// Fixed output of Grids so that the array and map parts can be recognized by
// loaddods.
//
// Revision 1.3  1996/10/23 23:46:39  jimg
// Modified so that writeval outputs a `recursive' data stream. Thus any
// data type can be written and later read without being flattened. This
// change was made so that the NSCAT data which is represented as a Sequence
// of Structures.
//
// Revision 1.2  1996/10/07 21:04:40  jimg
// Fixed Header.
// Added a write_val() that works.
//
// Revision 1.1  1996/09/30 23:59:06  jimg
// Added.
