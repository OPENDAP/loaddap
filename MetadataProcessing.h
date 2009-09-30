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

#include "DAS.h"
#include "DDS.h"
#include "BaseType.h"

using namespace libdap;

/** Combine DAS with DDS information for a dataset. The
 DDS is used to find the structure of the dataset (something that the DAS
 currently lacks). An instance of this class is first initialized using a
 DDS (which must contain variables meeting certain requirements described
 later) and then information is transferred from the dataset's DAS.
 Children of this class can then write this information, etc.

 The class also has methods which can remove extra variables currently
 added to the DDS by some servers and which can add a #size# attribute to
 a variable (which provides a way for other programs to learn about the
 dimensionality of a variable without first working with the DDS.

 This class depends on the DDS containing variables which inherit from
 both the DODS variable types (Byte, ..., Grid) and AttrTable. Methods of
 this class move information from the AttrTable objects of the DAS to the
 AttrTable objects of the variables in the DDS. In this way the structure
 of the attributes mirrors that of the dataset's author.

 This class' design is a test of the more general idea of combining the
 information currently spread between the DAS and DDS objects.

 @author jhrg
 @see DAS
 @see DDS
 @see write_dap */

class MetadataProcessing {
private:
protected:
	/** This DDS holds instances of variables which contain AttrTables. To
	 work, each variable must be an instance of a class which inherits
	 from AttrTable in addition to the DODS variable classes. */
	DDS meta_dds;

	/** Search, using names, for the attribute container that's got the same
	 name as the variable #btp#. This method is not very smart and will
	 not know what to do if two containers/variables have the same name. */
	void transfer_attr(DAS &das, BaseType &bt);

	bool is_global_attr(string name);

	/** Scan the DAS for global attributes (attribute containers which don't
	 match any top-level variable). Create a new variable called
	 #global_cont_name# and copy the global attribute containers to it.

	 @param das The DAS object from which to read attribute information.
	 @param global\_cont\_name The name to give the additional variable
	 which will hold the global attributes. */
	void add_global_attributes(DAS &das, string global_cont_name);

public:
	/** Create an instance based on the #dds#. Variables in the DDS must
	 inherit from both the DODS variable classes and AttrTable.

	 @param dds A DDS whose structure forms the basis for the combined DAS
	 and DDS information. */
	MetadataProcessing(DDS &dds);

	/** Using a dataset's DAS object, load the corresponding variables with
	 attribute information. This method assumes that the DDS contains
	 variables that inherit from both BaseType and AttrTable. The DAS
	 and DDS should be from the same dataset, but this constructor does
	 not check for that.
	 @param das The dataset's DAS object. */
	void transfer_attributes(DAS &das);
};
