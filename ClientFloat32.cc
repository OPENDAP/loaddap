
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

// Implementation for ClientFloat32. See ClientByte.cc
//
// 3/29/99 jhrg

#include "config_writedap.h"

#include <assert.h>
#include <iostream>
#include <iomanip>

#include <string>

#include "InternalErr.h"
#include "dods-limits.h"
#include "ClientFloat32.h"
#include "name_map.h"

extern name_map names;
extern bool translate;
extern bool ascii;

Float32 *
NewFloat32(const string &n)
{
    return new ClientFloat32(n);
}

ClientFloat32::ClientFloat32(const string &n) : Float32(n)
{
    set_matlab_name(n);
}

BaseType *
ClientFloat32::ptr_duplicate()
{
    return new ClientFloat32(*this);
}
 
bool
ClientFloat32::read(const string &) 
{
  throw InternalErr(__FILE__, __LINE__, "Called unimplemented read method");
}

void
ClientFloat32::print_val(FILE *os, string, bool print_decl_p)
{
    if (print_decl_p)
      fprintf(os, "%s\n%s\n", type_name().c_str(), get_matlab_name().c_str());

    if (translate) {
        dods_float64 df = _buf;
        if (ascii)
            fprintf(os, "%lg ", df);
        else
            fwrite((void *)&df, sizeof(dods_float32), 1, os);
    }
    else {
        if (ascii)
            fprintf(os, "%g ", (dods_float32)_buf);
        else
            fwrite((void *)&_buf, sizeof(dods_float32), 1, os);
    }   
}

AttrTable &
ClientFloat32::getAttrTable()
{
    return _attr;
}

void 
ClientFloat32::setAttrTable(AttrTable &attr)
{
    _attr = attr;
}

void
ClientFloat32::set_name(const string &n)
{
    BaseType::set_name(n);
    set_matlab_name(n);
}

string 
ClientFloat32::get_matlab_name() const
{
    return _matlabName; 
}

void 
ClientFloat32::set_matlab_name(const string &n)
{ 
    _matlabName = names.lookup(n, translate);
}

// $Log: ClientFloat32.cc,v $
// Revision 1.4  2004/07/08 20:50:03  jimg
// Merged release-3-4-5FCS
//
// Revision 1.3  2003/12/08 17:59:49  edavis
// Merge release-3-4 into trunk
//
// Revision 1.1.1.1  2003/10/22 19:43:17  dan
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
// Revision 1.8  2003/05/02 17:16:17  jimg
// I replaced the cast is ostream::write that was (void *) to (char *) to
// get this code to compile with gcc 3.2. This change is also needed for
// VC++, so I was able to remove some of the ifdef WIN32 lines. Also, in
// name_map I added a using namespace std; line.
//
// Revision 1.7  2001/08/27 18:06:57  jimg
// Merged release-3-2-5.
//
// Revision 1.6.2.1  2001/08/21 16:42:44  dan
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
// Revision 1.5  2000/07/21 10:21:56  rmorris
// Merged with win32-mlclient-branch.
//
// Revision 1.4.2.2  2000/07/09 22:26:32  rmorris
// Mod's to increase portability, minimize ifdef's for win32.
//
// Revision 1.4.2.1  2000/06/26 22:52:33  rmorris
// Mods for port to win32.
//
// Revision 1.4  2000/06/07 00:28:31  jimg
// Merged changes from version 3.1.4
//
// Revision 1.2.8.2  2000/06/02 23:03:31  jimg
// Fixes for AttrTables with no entries.
// Added support for name translation and a DODS_ML_Real_Name attribute.
// Changed the `size' attribtue's name to DODS_ML_Size.
//
// Revision 1.3  1999/11/09 06:43:40  jimg
// result of merge with 3-1-1
//
// Revision 1.2.8.1  1999/11/02 23:48:46  jimg
// Bug: used dods_float64 instead of dods_float32
//
// Revision 1.2  1999/04/30 17:06:54  jimg
// Merged with no-gnu and release-2-24
//
// Revision 1.1  1999/03/29 21:22:55  jimg
// Added
//
