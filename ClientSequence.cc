// -*- mode: c++; c-basic-offset: 4 -*-

// (c) COPYRIGHT URI/MIT 1996,2000
// Please read the full copyright statement in the file COPYRIGHT.
//
// Authors:
//      jhrg,jimg       James Gallagher (jgallagher@gso.uri.edu)

// Implementation for the class ClientStructure. See ClientByte.cc
//
// jhrg 9/26/96

#include "config_writedap.h"

#include <assert.h>

#include <iostream>
#include <Pix.h>
#include <string>

#include "InternalErr.h"
#include "debug.h"
#include "ClientSequence.h"
#include "name_map.h" 

extern name_map names;
extern bool translate;

extern void smart_newline(ostream &os, Type type);

Sequence *
NewSequence(const string &n)
{
    return new ClientSequence(n);
}

BaseType *
ClientSequence::ptr_duplicate()
{
    return new ClientSequence(*this);
}

ClientSequence::ClientSequence(const string &n) : Sequence(n)
{
    set_matlab_name(n);
}

ClientSequence::~ClientSequence()
{
}

bool 
ClientSequence::read(const string &)
{
    throw InternalErr(__FILE__, __LINE__, "Called unimplemented read method");
}

int
ClientSequence::length()
{
    return -1;
}

// Count the non-ctor elements in the Sequence recurring on embedded
// ctor types.

static int
count_elements(ClientSequence *sp, bool flat)
{
    int i = 0;

    for (Pix p = sp->first_var(); p; sp->next_var(p))
	switch (sp->var(p)->type()) {
	  case dods_byte_c:
	  case dods_int16_c:
	  case dods_uint16_c:
	  case dods_int32_c:
	  case dods_uint32_c:
	  case dods_float32_c:
	  case dods_float64_c:
	  case dods_str_c:
	  case dods_url_c:
	    i++;
	    break;

	  case dods_array_c:
	  case dods_list_c:
	  case dods_structure_c:
	  case dods_sequence_c:
	  case dods_grid_c:
	    if (flat)
		i += count_elements((ClientSequence *)sp->var(p), flat);
	    else
		i++;
	    break;

	  default:
	    return 0;
	}

    return i;
}

// Note: I replaced ClientSequence::print_val(...) with these two methods.
// Sequence::print_val(...) calls print_val_by_rows, so calling print_val
// winds up calling these methods. It's a little twisted, but no more than
// usual. 1/15/2002 jhrg
void 
ClientSequence::print_one_row(ostream &os, int row, string space,
			      bool print_row_num)
{
    os << type_name() << endl << get_matlab_name();

    int i = count_elements(this, false);
    os << " " << i << endl;

    const int elements = element_count();
    for (int j = 0; j < elements; ++j) {
	BaseType *bt_ptr = var_value(row, j);
	// We could throw here is bt_ptr is null, but lets keep going to see
	// if we can recover from the problem by continuing to stream the
	// data. 1/15/2002 jhrg
	if (bt_ptr) {		// data
	    bt_ptr->print_val(os, space, true);

            // Add a new-line if the contained variable is an Array type.
            // This is required so that sequences containing Grids/Arrays
            // will work the same as simple datasets containing Grids/Arrays.
            // 1/15/2003 danh
            switch ( bt_ptr->type() ) {
            case dods_array_c:
		os << endl;
		break;
	    
	    }
	}	
    }

}

// Sequence::print_val(...) passes false for the print_row_numbers parameter.
void 
ClientSequence::print_val_by_rows(ostream &os, string space,
				  bool print_decl_p,
				  bool print_row_numners)
{
    const int rows = number_of_rows();
    for (int i = 0; i < rows; ++i) {
	print_one_row(os, i, space, false);
    }
    os << "EndSequence" << endl;
}

AttrTable &
ClientSequence::getAttrTable()
{
    return _attr;
}

void 
ClientSequence::setAttrTable(AttrTable &attr)
{
    _attr = attr;
}

void
ClientSequence::set_name(const string &n)
{
    BaseType::set_name(n);
    set_matlab_name(n);
}

string 
ClientSequence::get_matlab_name() const
{
    return _matlabName; 
}

void 
ClientSequence::set_matlab_name(const string &name)
{ 
    _matlabName = names.lookup(name, translate);
}

// $Log: ClientSequence.cc,v $
// Revision 1.4  2004/07/08 20:50:03  jimg
// Merged release-3-4-5FCS
//
// Revision 1.1.1.1  2003/10/22 19:43:19  dan
// Version of the Matlab CommandLine client which uses Matlab Structure
// variables to maintain the shape of the underlying DODS data.
// Revision 1.1.1.1.2.3  2004/03/08 17:52:49  dan
// Modified to support storing Sequence variables
// in Matlab structures.   This required handling
// arrays and recognizing an EndofSequence marker
// in the deserialization stream.
//
// Revision 1.3  2003/12/08 17:59:49  edavis
// Merge release-3-4 into trunk
//
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
// Revision 1.25  2003/04/22 14:40:29  dan
// Removed changes added to maintain DDS structure, these
// changes have been placed on the structure-format1 branch
// until the GUI is ready to use them.
//
// Revision 1.23  2003/01/29 15:43:52  dan
// Resolved conflict on merge, caused by PWEST's removal
// of the SLLIST includes in a previous update on the trunk.
//
// Revision 1.22  2001/08/27 18:06:57  jimg
// Merged release-3-2-5.
//
// Revision 1.21.2.5  2002/09/05 22:26:49  pwest
// Removed includes to SLList and DLList. These are not necessary and no longer
// supported.
//
// Revision 1.21.2.4  2002/01/28 09:16:57  rmorris
// Removed default arg definitions.  Redundant with the .h and VC++
// chokes on these redundancies.  For print_one_row() and print_val_by_rows().
//
// Revision 1.21.2.3  2002/01/16 03:01:43  jimg
// Removed a call to `smart_newline.'
//
// Revision 1.21.2.2  2002/01/16 00:32:32  jimg
// A while ago I changed the way Sequences are deserialized in the DAP. To
// accommodate these changes the code in ClientSequence, ClientStructure and
// process_data had to be fixed.
// NB: It would be better to not use BaseType::print_val(...) but instead
// use a mixin for writing out writeval's stuff. See the asciival code for
// an example.
//
// Revision 1.21.2.1  2001/08/21 16:48:41  dan
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
// Revision 1.21  2000/10/03 22:24:17  jimg
// Reorganized the CVS Log entries.
// Changed the read() mfunc definitions to match the 3.2 dap++ library.
//
// Revision 1.20  2000/07/21 10:21:56  rmorris
// Merged with win32-mlclient-branch.
//
// Revision 1.19.2.2  2000/07/09 22:26:32  rmorris
// Mod's to increase portability, minimize ifdef's for win32.
//
// Revision 1.19.2.1  2000/06/26 22:54:26  rmorris
// Mods for port to win32.
//
// Revision 1.19  2000/06/07 00:28:31  jimg
// Merged changes from version 3.1.4
//
// Revision 1.18.8.1  2000/06/02 23:03:32  jimg
// Fixes for AttrTables with no entries.
// Added support for name translation and a DODS_ML_Real_Name attribute.
// Changed the `size' attribtue's name to DODS_ML_Size.
//
// Revision 1.18  1999/04/30 17:06:55  jimg
// Merged with no-gnu and release-2-24
//
// Revision 1.17  1999/03/24 06:23:43  brent
// convert String.h to std lib <string>, convert to packages regex -- B^2
//
// Revision 1.16  1998/11/20 08:41:22  jimg
// Fixed the fix for the extra newline separators; it was broken for arrays
//
// Revision 1.15  1998/11/19 20:53:33  jimg
// Fixed the extra newline bug.
//
// Revision 1.14  1998/09/16 23:02:21  jimg
// Removed write_val() and replaced it with print_val() and print_all_vals()
//
// Revision 1.13  1998/08/03 16:39:49  jimg
// Fixed write_val mfunc so that outermost is no longer used
//
// Revision 1.12  1998/06/09 04:08:02  jimg
// Fixed a but in write_val where deserialize was not called at the top of
// the loop that reads values.
//
// Revision 1.11  1997/10/04 00:40:05  jimg
// Added changes to support the new deserialize() mfunc and write_val().
//
// Revision 1.10  1997/07/16 00:29:12  jimg
// Fixed Sequences::length return type
//
// Revision 1.9  1997/04/19 00:54:56  jimg
// Changed ouput stream to simplify reading (for loaddods).
//
// Revision 1.8  1997/02/06 20:43:40  jimg
// Fixed log messages
//
// Revision 1.7  1997/01/10 06:49:36  jimg
// Changed call to name_map::lookup() so that non-alphanumerics are mapped to
// underscore. 
//
// Revision 1.6  1996/11/23 05:12:11  jimg
// Added support for variable renaming via the name_map object.
//
// Revision 1.5  1996/10/29 21:43:21  jimg
// Always call smart_newline after each field of a Sequence
//
// Revision 1.4  1996/10/25 23:09:43  jimg
// Fixed count_elements so that it works correctly depending on how FLATTEN is
// defined.
// Fixed write_val.
//
// Revision 1.3  1996/10/23 23:46:43  jimg
// Modified so that writeval outputs a `recursive' data stream. Thus any
// data type can be written and later read without being flattened. This
// change was made so that the NSCAT data which is represented as a Sequence
// of Structures.
//
// Revision 1.2  1996/10/07 21:02:33  jimg
// Fixed Header.
// Replaced code in write_val with static functions (which will work better
// once the software handles Sequences with ctor types).
//
// Revision 1.1  1996/09/30 23:59:15  jimg
// Added.
