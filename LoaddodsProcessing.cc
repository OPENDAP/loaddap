
// (c) COPYRIGHT URI 2000
// Please read the full copyright statement in the file COPYRIGHT.
//
// Authors:
//	jhrg,jimg	James Gallagher (jgallagher@gso.uri.edu)

// The implementation of the MetadataProcessing class. 

#include <exception>
#include <typeinfo>
#include <iostream>
#include <strstream>

#include "debug.h"

#include "Pix.h"

#include "dods-datatypes.h"
#include "InternalErr.h"

#include "name_map.h"
#include "LoaddodsProcessing.h"
#include "ClientArray.h"
#include "ClientStructure.h"
#include "ClientSequence.h"
#include "ClientGrid.h"
#include "AttributeInterface.h"

#include <dods-datatypes.h>

using namespace std;

// The flag `translate' says whether we are to perform DODS--> Matlab
// namespace transformations. This generally means removing %20 sequences and
// turning them into underscores. When this option is on, we must add an
// attribute called DODS_Real_Name that holds the exact text of the
// variable's real name. This is so that a ML client can build a URL using
// the string to ask for the variable. 6/2/2000 jhrg
extern name_map names;
extern bool translate;

void
LoaddodsProcessing::print_attr_table(AttrTable &at, ostream &os)
{
    for (Pix p = at.first_attr(); p; at.next_attr(p)) {
	int attr_num = at.get_attr_num(p);
	switch (at.get_attr_type(p)) {
	  case Attr_container: {
	      AttrTable *cont_atp = at.get_attr_table(p);
	      os << "Structure" << endl << names.lookup(at.get_name(p), translate)
		 << " " << cont_atp->get_size() << endl;
	      print_attr_table(*cont_atp, os);
	      break;
	  }
	  case Attr_string:
	  case Attr_url:
	    if (attr_num == 1) {
		os << "String" << endl << names.lookup(at.get_name(p), translate) << endl
		   << at.get_attr(p) << endl;
	    }
	    else {
		os << "Array" << endl << "String " << names.lookup(at.get_name(p), translate)
		   << " 1" << endl << attr_num << endl;

		for (int i = 0; i < attr_num; ++i)
		  os << at.get_attr(p, i) << endl;
#if 0
		  os << at.get_name(p) << endl << at.get_attr(p, i) << endl;
#endif
		
		// Send a trailing newline character
		os << endl;
	    }
	    break;
	  default:
	    if (attr_num == 1) {
		os << "Float64" << endl << names.lookup(at.get_name(p), translate) << endl;
		dods_float64 df = atof(at.get_attr(p).c_str());
		os.write((char *)&df, sizeof(dods_float64));
		os << endl;
	    }
	    else {
		os << "Array" << endl << "Float64 " << names.lookup(at.get_name(p), translate) 
		   << " 1" << endl << attr_num << endl;
		for (int i = 0; i < attr_num; ++i) {
		    dods_float64 df = atof(at.get_attr(p, i).c_str());
		    os.write((char *)&df, sizeof(dods_float64));
		}
		os << endl;
	    }
	    break;
	}
    }
}

// The number of attributes is, from the Matlab view, the number of entries
// in the AttrTable plus the number of variables contained by this variable.
// For example, a Grid variable with three of its own attributes, one array
// and two map vectors would have a total of six `attributes'. That's because
// each of the contained variables (the array and the two maps) are
// themselves elements of the structure used to represent the Grid to Matlab.
// If the structure used to represent the Grid does not have entries for the
// array and maps then Matlab won't know about them.
static int
number_of_attributes(BaseType &bt)
{
    AttributeInterface &ai = dynamic_cast<AttributeInterface &>(bt);
    AttrTable &at = ai.getAttrTable();
    int i = at.get_size();

    switch (bt.type()) {
      case dods_structure_c:
      case dods_sequence_c:
      case dods_grid_c:
	  i += bt.element_count();
	  break;
      default:
	break;
    }

    return i;
}

void
LoaddodsProcessing::print_attributes(BaseType &bt, ostream &os)
{
    // Each variable is sent to matlab as a structure so that loaddods will
    // know to make a ML structure to hold the size and attributes.

    os << "Structure" << endl;

    try {
	// Count the number of attributes; write out the variable name and the
	// number of attributes.

	os << names.lookup(bt.name(), translate) << " " 
	   << number_of_attributes(bt) << endl;

	// Write out this variable's attribute table.
	AttributeInterface &ai = dynamic_cast<AttributeInterface &>(bt);
	AttrTable &at = ai.getAttrTable();
	print_attr_table(at, os);

	// For each variable contained by this variable, write out its
	// attribute table.
	switch (bt.type()) {
	  case dods_structure_c:  {
	      Structure &s = dynamic_cast<Structure &>(bt);
	      for (Pix p = s.first_var(); p; s.next_var(p))
		  print_attributes(*s.var(p), os);
	      break;
	  }
	  case dods_sequence_c:  {
	      Sequence &s = dynamic_cast<Sequence &>(bt);
	      for (Pix p = s.first_var(); p; s.next_var(p))
		  print_attributes(*s.var(p), os);
	      break;
	  }
	  case dods_grid_c: {
	      Grid &g = dynamic_cast<Grid &>(bt);
	      print_attributes(*g.array_var(), os);
	      for (Pix p = g.first_map_var(); p; g.next_map_var(p))
		  print_attributes(*g.map_var(p), os);
	      break;
	  }
	  default:
	    break;
	}
	
    }
    catch (bad_cast &bc) {
	throw InternalErr(__FILE__, __LINE__,
"Object in cast was either not an AttrTable or was not a BaseType object could not be downcast.");
    }
}

LoaddodsProcessing::LoaddodsProcessing()
{
}

LoaddodsProcessing::LoaddodsProcessing(DDS &dds) : MetadataProcessing(dds)
{
}

// Only handles variable's attributes; need to handle global attributes.
void 
LoaddodsProcessing::print_for_matlab(ostream &os)
{
    // Write the `Attribute' keyword and number of top level variables.
    os << "Attributes" << endl 
       << meta_dds.get_dataset_name() << " "
       << meta_dds.num_var() << endl;

    // Write information about each top level varaible. This prints all the
    // variables, so if some are to be excluded, they must be removed from
    // the DDS before this is run.
    for (Pix p = meta_dds.first_var(); p; meta_dds.next_var(p)) {
	BaseType *btp = meta_dds.var(p);
	print_attributes(*btp, os);
    }
}

// For every Grid, compare the names of all the map vectors. Any match
// causes this function to return true.
static bool
is_replicate_map_vector(BaseType &bt, DDS &dds)
{
    for (Pix p = dds.first_var(); p; dds.next_var(p)) {
	BaseType &var = *dds.var(p);
	if (var.type() == dods_grid_c) {
	    Grid &g = dynamic_cast<Grid &>(var);
	    for (Pix q = g.first_map_var(); q; g.next_map_var(q))
		if (g.map_var(q)->name() == bt.name())
		    return true;
	}
    }

    return false;
}

// Scan the dataset and look for toplevel Array valriables that are also
// map vectors of a Grid. Remove the toplevel Array variables. Note that the
// Grids must themselves be at the toplevel, so their map vectors are one
// level down from the top.
void 
LoaddodsProcessing::prune_duplicates()
{
    BaseType *bt;
    for (Pix p = meta_dds.first_var(); p; meta_dds.next_var(p)) {
	bt = meta_dds.var(p);
	DBG(cerr << "Looking at: " << bt->name() << " (" << bt << ")" << endl);
	if (bt->type() == dods_array_c 
	    && is_replicate_map_vector(*bt, meta_dds)) {
	    DBG(cerr << "Deleting: " << bt->name() << " (" << bt << ")" << endl);
	    meta_dds.del_var(bt->name()); // only deletes at the top level!
	}
	DBG(cerr << "Looking past: " << endl);
    }
}

static void
add_size_attr(BaseType &bt)
{
    try {
	switch (bt.type()) {
	  case dods_array_c:  {
	      ClientArray &a = dynamic_cast<ClientArray &>(bt);
	      AttrTable &at = a.getAttrTable();
	      for (Pix p = a.first_dim(); p; a.next_dim(p)) {
		  ostrstream oss;
		  oss << a.dimension_size(p) << ends;
		  at.append_attr("DODS_ML_Size", "Float64", oss.str());
		  oss.freeze(0);
	      }
	      break;
	  }
	  case dods_grid_c: {
	      ClientGrid &g = dynamic_cast<ClientGrid &>(bt);
	      add_size_attr(*g.array_var());
	      for (Pix p = g.first_map_var(); p; g.next_map_var(p))
		  add_size_attr(*g.map_var(p));
	      break;
	  }
	  case dods_structure_c:  {
	      ClientStructure &s = dynamic_cast<ClientStructure &>(bt);
	      for (Pix p = s.first_var(); p; s.next_var(p))
		  add_size_attr(*s.var(p));
	      break;
	  }
	  case dods_sequence_c:  {
	      ClientSequence &s = dynamic_cast<ClientSequence &>(bt);
	      for (Pix p = s.first_var(); p; s.next_var(p))
		  add_size_attr(*s.var(p));
	      break;
	  }
	  default:
	    break;
	}
    }
    catch (bad_cast &bc) {
	throw InternalErr(__FILE__, __LINE__,
			  "Either a BaseType object could not be downcast or could not be cast to AttributeInterface.");
    }
}

void 
LoaddodsProcessing::add_size_attributes()
{
    for (Pix p = meta_dds.first_var(); p; meta_dds.next_var(p)) {
	BaseType &bt = *meta_dds.var(p);
	add_size_attr(bt);
    }
}

static void
add_realname_attr(BaseType &bt)
{
    AttrTable &at = dynamic_cast<AttributeInterface &>(bt).getAttrTable();
    at.append_attr("DODS_ML_Real_Name", "String", bt.name());

    try {
	switch (bt.type()) {
	  case dods_grid_c: {
	      ClientGrid &g = dynamic_cast<ClientGrid &>(bt);
	      add_realname_attr(*g.array_var());
	      for (Pix p = g.first_map_var(); p; g.next_map_var(p))
		  add_realname_attr(*g.map_var(p));
	      break;
	  }
	  case dods_structure_c:  {
	      ClientStructure &s = dynamic_cast<ClientStructure &>(bt);
	      for (Pix p = s.first_var(); p; s.next_var(p))
		  add_realname_attr(*s.var(p));
	      break;
	  }
	  case dods_sequence_c:  {
	      ClientSequence &s = dynamic_cast<ClientSequence &>(bt);
	      for (Pix p = s.first_var(); p; s.next_var(p))
		  add_realname_attr(*s.var(p));
	      break;
	  }
	  default:
	    break;
	}
    }
    catch (bad_cast &bc) {
	throw InternalErr(__FILE__, __LINE__,
			  "Either a BaseType object could not be downcast or could not be cast to AttributeInterface.");
    }
}

void 
LoaddodsProcessing::add_realname_attributes()
{
    if (translate)
	for (Pix p = meta_dds.first_var(); p; meta_dds.next_var(p)) {
	    BaseType &bt = *meta_dds.var(p);
	    add_realname_attr(bt);
	}
}

// $Log: LoaddodsProcessing.cc,v $
// Revision 1.2  2003/12/08 17:59:49  edavis
// Merge release-3-4 into trunk
//
// Revision 1.1.1.1  2003/10/22 19:43:32  dan
// Version of the Matlab CommandLine client which uses Matlab Structure
// variables to maintain the shape of the underlying DODS data.
//
// Revision 1.11  2003/05/02 17:16:17  jimg
// I replaced the cast is ostream::write that was (void *) to (char *) to
// get this code to compile with gcc 3.2. This change is also needed for
// VC++, so I was able to remove some of the ifdef WIN32 lines. Also, in
// name_map I added a using namespace std; line.
//
// Revision 1.10  2003/04/22 14:43:41  dan
// Removed changes added to maintain DDS structure, these
// changes have been placed on the structure-format1 branch
// until the GUI is ready to use them.
//
// Revision 1.8  2003/01/29 16:18:14  dan
// Merged with release-3-2-7
//
// Revision 1.7.2.1  2002/08/20 13:40:41  dan
// Use names.translate() method to insure that
// attributes (which are being converted to variables)
// conform to the Matlab variable naming convention.
//
// Revision 1.7  2000/10/03 22:24:17  jimg
// Reorganized the CVS Log entries.
// Changed the read() mfunc definitions to match the 3.2 dap++ library.
//
// Revision 1.6  2000/08/30 00:13:29  jimg
// Merged with 3.1.7
//
// Revision 1.1.2.6  2000/08/21 19:53:08  jimg
// Changed the way string arrays found in attribute objects are externalized.
// This now matches the way its done in ClientArray.
//
// Revision 1.1.2.5 2000/08/17 23:51:21 jimg Fixed writing of arrays of
// strings. It now matches what loaddods expects, or at least, it matches the
// grammar as documented. loaddods still cannot read it correctly, but that
// is loaddods' problem.
//
// Revision 1.5  2000/08/08 20:47:21  rmorris
// Added include of a new dods header - dods-datatypes.h
//
// Revision 1.4  2000/07/21 10:21:56  rmorris
// Merged with win32-mlclient-branch.
//
// Revision 1.3.2.1  2000/06/26 23:02:21  rmorris
// Modification for port to win32.
//
// Revision 1.3  2000/06/15 00:01:32  jimg
// Merged with 3.1.5
//
// Revision 1.1.2.4  2000/06/14 20:43:08  jimg
// Added instrumentation in prune_duplicates().
//
// Revision 1.2  2000/06/07 00:28:31  jimg
// Merged changes from version 3.1.4
//
// Revision 1.1.2.3  2000/06/02 23:03:32  jimg
// Fixes for AttrTables with no entries.
// Added support for name translation and a DODS_ML_Real_Name attribute.
// Changed the `size' attribtue's name to DODS_ML_Size.
//
// Revision 1.1.2.2  2000/05/31 23:16:45  jimg
// Fixed errors WRT the documented output.
//
// Revision 1.1.2.1  2000/05/26 20:37:25  jimg
// Created
//
