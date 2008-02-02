
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

// (c) COPYRIGHT URI/MIT 1999
// Please read the full copyright statement in the file COPYRIGHT.
//
// Authors:
//      jhrg,jimg       James Gallagher (jgallagher@gso.uri.edu)

// Interface for ClientFloat32 type. See ClientByte.h

#ifndef _clientfloat32_h
#define _clientfloat32_h 1

#include "Float32.h"
#include "AttrTable.h"
#include "AttributeInterface.h"

using namespace libdap ;

class ClientFloat32: public Float32, public AttributeInterface {
private:
    AttrTable _attr;
    string _matlabName;

public:
    ClientFloat32(const string &n = (char *)0);
    virtual ~ClientFloat32() {}

    virtual BaseType *ptr_duplicate();
    
    virtual bool read(const string &dataset);

    virtual void print_val(FILE *os, string space = "", 
			   bool print_decl_p = true);

    virtual AttrTable &getAttrTable();
    virtual void setAttrTable(AttrTable &attr);

    virtual void set_name(const string &n);

    virtual string get_matlab_name() const;
    virtual void set_matlab_name(const string &n);
};

// $Log: ClientFloat32.h,v $
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
// Revision 1.6  2001/08/27 18:06:57  jimg
// Merged release-3-2-5.
//
// Revision 1.5.2.1  2001/08/21 16:42:52  dan
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
// Revision 1.5  2000/10/03 22:24:17  jimg
// Reorganized the CVS Log entries.
// Changed the read() mfunc definitions to match the 3.2 dap++ library.
//
// Revision 1.4  2000/07/21 10:21:56  rmorris
// Merged with win32-mlclient-branch.
//
// Revision 1.3.2.2  2000/07/09 22:21:22  rmorris
// Mod's to increase portability and to minimize ifdef's for win32.
//
// Revision 1.3.2.1  2000/06/26 22:52:33  rmorris
// Mods for port to win32.
//
// Revision 1.3  2000/06/07 00:28:31  jimg
// Merged changes from version 3.1.4
//
// Revision 1.2.8.2  2000/06/02 23:03:31  jimg
// Fixes for AttrTables with no entries.
// Added support for name translation and a DODS_ML_Real_Name attribute.
// Changed the `size' attribtue's name to DODS_ML_Size.
//
// Revision 1.2.8.1  2000/05/12 19:14:15  jimg
// Now inherits from AttrTable, too
//
// Revision 1.2  1999/04/30 17:06:54  jimg
// Merged with no-gnu and release-2-24
//
// Revision 1.1  1999/03/29 21:22:56  jimg
// Added
//

#endif

