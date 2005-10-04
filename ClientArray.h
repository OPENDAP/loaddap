
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

// (c) COPYRIGHT URI/MIT 1996,2000
// Please read the full copyright statement in the file COPYRIGHT.
//
// Authors:
//      jhrg,jimg       James Gallagher (jgallagher@gso.uri.edu)

// Interface definition for ClientArray. See ClientByte.h for more information
//
// jhrg 1/12/95

#ifndef _clientarray_h
#define _clientarray_h 1

#include "Array.h"
#include "AttributeInterface.h"

/** This class provides access to Array data retrieved using the DAP
    protocol. The DAP protocol can be used to transport data values as well
    as metadata. Using the AttributeInterface of this class, metadata from
    the DAP DAS object can be merged with the data. Other clients of this
    class can then have ready access to both the data and associated metadata
    for an Array variable. 

    @see Array
    @see AttributeInterface
    @see AttrTable
    @author jhrg */
class ClientArray: public Array, public AttributeInterface {
private:
    AttrTable _attr;
    string _matlabName;

public:
    /** Create an instance of this class.
	@param n The name of the variable.
	@param v A template for the data type of elements of the array. */
    ClientArray(const string &n = (char *)0, BaseType *v = 0);

    /** Destroy an instance of this class. */
    virtual ~ClientArray();

    /** Duplicate this instance and return a pointer to the duplicate. The
	duplicate is dynamically alloaced using new and must be freed using
	delete. 
	@return BaseType* */
    virtual BaseType *ptr_duplicate();

    /** The read method for this class always returns false; this class is
	designed for client-side use only. */
    virtual bool read(const string &dataset);

    /** Print the array values. This method prints the values using a grammar
	design to encapsilate all the semantics of the DODS data types
	while being very simple to parse. See the design documentation for
	loaddods for the productions of this grammar.
	@param os Print to this stream.
	@param space Unused by this version of print\_val.
	@param print\_decl\_p Unused. */
    virtual void print_val(ostream &os, string space = "", 
			   bool print_decl_p = true);

    /** Get this instances attribute table.
	@return AttrTable \& */
    virtual AttrTable &getAttrTable();

    /** Set this instances attribute table.
	@param attr An AttrTable object which contains attribute information
	about this instance. */
    virtual void setAttrTable(AttrTable &attr);

    virtual void set_name(const string &n);

    virtual string get_matlab_name() const;
    virtual void set_matlab_name(const string &n);
};

// $Log: ClientArray.h,v $
// Revision 1.2  2003/12/08 17:59:49  edavis
// Merge release-3-4 into trunk
//
// Revision 1.1.1.1.2.1  2003/10/29 19:03:21  dan
// Removed 'pragma interface' directive from all subclass
// source files.
//
// Revision 1.1.1.1  2003/10/22 19:43:17  dan
// Version of the Matlab CommandLine client which uses Matlab Structure
// variables to maintain the shape of the underlying DODS data.
//
// Revision 1.12  2001/08/27 18:06:57  jimg
// Merged release-3-2-5.
//
// Revision 1.11.2.1  2001/08/21 16:41:51  dan
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
// Revision 1.11  2000/10/03 22:24:17  jimg
// Reorganized the CVS Log entries.
// Changed the read() mfunc definitions to match the 3.2 dap++ library.
//
// Revision 1.10  2000/07/21 10:21:55  rmorris
// Merged with win32-mlclient-branch.
//
// Revision 1.9  2000/06/20 21:40:08  jimg
// Merged with 3.1.6
//
// Revision 1.8.2.2  2000/07/09 22:21:22  rmorris
// Mod's to increase portability and to minimize ifdef's for win32.
//
// Revision 1.8.2.1  2000/06/26 22:49:32  rmorris
// Changes for port to win32.
//
// Revision 1.9  2000/06/20 21:40:08  jimg
// Merged with 3.1.6
//
// Revision 1.7.8.3  2000/06/20 20:36:45  jimg
// Imporved error messages and fixed some doc++ comments (although some
// problems remain).
//
// Revision 1.8  2000/06/07 00:28:31  jimg
// Merged changes from version 3.1.4
//
// Revision 1.7.8.2  2000/06/02 23:03:31  jimg
// Fixes for AttrTables with no entries.
// Added support for name translation and a DODS_ML_Real_Name attribute.
// Changed the `size' attribtue's name to DODS_ML_Size.
//
// Revision 1.7.8.1  2000/05/12 19:14:15  jimg
// Now inherits from AttrTable, too
//
// Revision 1.7  1999/04/30 17:06:54  jimg
// Merged with no-gnu and release-2-24
//
// Revision 1.6  1998/09/16 23:02:23  jimg
// Removed write_val() and replaced it with print_val() and print_all_vals()
//
// Revision 1.5  1998/08/03 16:39:44  jimg
// Fixed write_val mfunc so that outermost is no longer used
//
// Revision 1.4  1997/10/04 00:39:59  jimg
// Added changes to support the new deserialize() mfunc and write_val().
//
// Revision 1.3  1996/10/23 23:46:48  jimg
// Modified so that writeval outputs a `recursive' data stream. Thus any
// data type can be written and later read without being flattened. This
// change was made so that the NSCAT data which is represented as a Sequence
// of Structures.
//
// Revision 1.2  1996/10/07 21:00:33  jimg
// Fixed Headers.
//
// Revision 1.1  1996/09/30 23:58:51  jimg
// Added.

#endif

