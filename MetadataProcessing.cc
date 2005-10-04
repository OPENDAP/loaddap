
// -*- mode: c++; c-basic-offset:4 -*-

// This file is part of loaddap.


// Copyright (c) 2005 OPeNDAP, Inc.
// Author: James Gallagher <jgallagher@opendap.org>
//
// This is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
// 
// This software is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
// You can contact OPeNDAP, Inc. at PO Box 112, Saunderstown, RI. 02874-0112.

// (c) COPYRIGHT URI 2000
// Please read the full copyright statement in the file COPYRIGHT.
//
// Authors:
//	jhrg,jimg	James Gallagher (jgallagher@gso.uri.edu)

// The implementation of the MetadataProcessing class. 

#include <exception>
#include <typeinfo>
#include <iostream>

#include <debug.h>

#include <GNURegex.h>

#include "name_map.h"
#include "MetadataProcessing.h"
#include "Error.h"
#include "InternalErr.h"
#include "ClientArray.h"
#include "ClientStructure.h"
#include "ClientSequence.h"
#include "ClientGrid.h"

// The flag `translate' says whether we are to perform DODS--> Matlab
// namespace transformations. This generally means removing %20 sequences and
// turning them into underscores. When this option is on, we must add an
// attribute called DODS_Real_Name that holds the exact text of the
// variable's real name. This is so that a ML client can build a URL using
// the string to ask for the variable. 6/2/2000 jhrg
extern name_map names;
extern bool translate;

void
MetadataProcessing::transfer_attr(DAS &das, BaseType &bt)
{
    // The nesting of the try/catch blocks below separates the possible error
    // of casting to AttrTable from the later casts to Structure, etc.
    try {
	// We know that this is one of the Client* specializations of
	// BaseType and we know that those also inherit from AttrTable. Cast
	// laterally to AttrTable.
	DBG(cerr << " About to cast a " << bt.type_name() << endl);

	AttributeInterface &ai = dynamic_cast<AttributeInterface &>(bt);
	AttrTable &at = ai.getAttrTable();

	try {
	    // Use the variable's name to find the correct attribtue table in
	    // the DAS object.
	    string name = bt.name();
	    AttrTable *das_attrs = das.get_table(name);

	    // Copy attributes from the DAS to the AttrTable that's part of
	    // the variable in the DDS.
	    if (das_attrs)
		ai.setAttrTable(*das_attrs);

	    // If this is a ctor type, call for each child.
	    switch (bt.type()) {
	      case dods_structure_c: {
		  Structure &s = dynamic_cast<Structure &>(bt);
		  Constructor::Vars_iter p;
		  for (p = s.var_begin(); p != s.var_end(); ++p)
		      transfer_attr(das, **p);
		  break;
	      }
	      case dods_sequence_c: {
		  Sequence &s = dynamic_cast<Sequence &>(bt);
		  Constructor::Vars_iter p;
		  for (p = s.var_begin(); p != s.var_end(); ++p)
		      transfer_attr(das, **p);
		  break;
	      }
	      case dods_grid_c: {
		  Grid &g = dynamic_cast<Grid &>(bt);
		  transfer_attr(das, *g.array_var());
		  Grid::Map_iter p;
		  for (p = g.map_begin(); p != g.map_end(); ++p)
		      transfer_attr(das, **p);
		  break;
	      }	  
	      default:
		break;
	    }
	}
	catch (bad_cast &bc) {
	    throw InternalErr(__FILE__, __LINE__,
			      "A BaseType object could not be downcast.\n");
	}
    }
    catch (bad_cast &bc) {
	throw InternalErr(__FILE__, __LINE__,
			  "Object in cast was not an AttributeInterface.");
    }

    // What's up?
    // DBG2(btp->print_decl(cerr));
    // DBG2(atp->print(cerr));
}

// An attribute is global if it's name does not match any of the top-level
// variables. 
bool
MetadataProcessing::is_global_attr(string name)
{
    DDS::Vars_iter p;
    for (p = meta_dds.var_begin(); p != meta_dds.var_end(); ++p)
	if ((*p)->name() == name)
	    return false;

    return true;
}

// This code could use a real `kill-file' some day - about the same time that
// the rest of the server gets a `rc' file... For the present just see if a
// small collection of regexs match the name.
static bool
is_in_kill_file(const string &name)
{
    static Regex dim(".*_dim_[0-9]*", 1); // HDF `dimension' attributes.

    return dim.match(name.c_str(), name.length()) != -1;
}

// For each global attribute container, add a variable with the same name as
// the container to the new structure. Then add the global container to that
// new structure element.
void
MetadataProcessing::add_global_attributes(DAS &das, string global_cont_name)
{
    ClientStructure *cs = new ClientStructure(global_cont_name);

    AttrTable::Attr_iter p;
    for (p = das.attr_begin(); p != das.attr_end(); ++p) {
	string name = das.get_name(p);

	if (!is_in_kill_file(name) && is_global_attr(name)) {
	    AttrTable *attr = das.get_table(p);
	    // Why ClientStructure? Why not? Any type will do since it never
	    // gets a value. We just need something in the DDS to hold the
	    // global attributes so they can be output using the same code as
	    // for the real variables. 6/2/2000 jhrg
	    ClientStructure *cb = new ClientStructure(name);
	    cb->setAttrTable(*attr);
	    cs->add_var(cb);
	}
    }
    meta_dds.add_var(cs);
}

MetadataProcessing::MetadataProcessing()
{
}

MetadataProcessing::MetadataProcessing(DDS &dds): meta_dds(dds)
{
}

void 
MetadataProcessing::transfer_attributes(DAS &das)
{
    DDS::Vars_iter p;

    for (p = meta_dds.var_begin(); p != meta_dds.var_end(); ++p) {
	BaseType *btp = (*p);
	transfer_attr(das, *btp);
    }

    add_global_attributes(das, "Global_Attributes");
}

// $Log: MetadataProcessing.cc,v $
// Revision 1.3  2004/02/06 00:50:18  jimg
// Removed strstream include.
//
// Revision 1.2  2003/12/08 17:59:49  edavis
// Merge release-3-4 into trunk
//
// Revision 1.1.1.1  2003/10/22 19:43:33  dan
// Version of the Matlab CommandLine client which uses Matlab Structure
// variables to maintain the shape of the underlying DODS data.
//
// Revision 1.5  2003/01/29 16:18:14  dan
// Merged with release-3-2-7
//
// Revision 1.4.2.1  2003/01/02 16:38:57  dan
// Modified add_global_attributes changing global attribute
// container name from 'global' to 'Global_Attributes'.  Fixed
// problem in add_global_attributes to insure that global
// attributes were placed correctly in the global container.
//
// Revision 1.4  2000/10/03 22:24:17  jimg
// Reorganized the CVS Log entries.
// Changed the read() mfunc definitions to match the 3.2 dap++ library.
//
// Revision 1.3  2000/07/21 10:21:56  rmorris
// Merged with win32-mlclient-branch.
//
// Revision 1.2.2.2  2000/07/09 22:26:33  rmorris
// Mod's to increase portability, minimize ifdef's for win32.
//
// Revision 1.2.2.1  2000/06/26 23:02:21  rmorris
// Modification for port to win32.
//
// Revision 1.2  2000/06/07 00:28:31  jimg
// Merged changes from version 3.1.4
//
// Revision 1.1.2.4  2000/06/02 23:03:33  jimg
// Fixes for AttrTables with no entries.
// Added support for name translation and a DODS_ML_Real_Name attribute.
// Changed the `size' attribtue's name to DODS_ML_Size.
//
// Revision 1.1.2.3  2000/06/01 01:32:52  jimg
// Added special code for global attributes
//
// Revision 1.1.2.2  2000/05/26 20:38:03  jimg
// Removed ML Specific code to LoaddodsProcessing.
//
