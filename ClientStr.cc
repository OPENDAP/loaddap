
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

// (c) COPYRIGHT URI/MIT 1996
// Please read the full copyright statement in the file COPYRIGH.  
//
// Authors:
//      jhrg,jimg       James Gallagher (jgallagher@gso.uri.edu)

// Implementation for ClientStr. See ClientByte.cc
//
// jhrg 9/25/96

#include <assert.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>

#include "config_writedap.h"	// was first caused LITTLE_ENDIAN error

#include <iostream>
#include <string>

#include <GNURegex.h>

#include "InternalErr.h"
#include "ClientStr.h"
#include "name_map.h"

// We *should* be using <nan.h> to handle generating NaN. However, many
// machines don't have a function that can be used to generate NaN so I've
// written my own (which uses its own bastardized version of the FP union).
// 2/5/97 jhrg

#include "nan_hack.h"

static double
MakeNaN() 
{ 
    dnan X;
    X.nan.exponent = 0x7ff; 
    X.nan.qnan_bit = 0x1; 
    
    return X.d;
}

extern name_map names;
extern bool translate;
extern bool ascii;
extern bool string_to_float;
extern bool verbose;
extern bool warning;

Str *
NewStr(const string &n)
{
    return new ClientStr(n);
}

ClientStr::ClientStr(const string &n) : Str(n)
{
    set_matlab_name(n);
}

BaseType *
ClientStr::ptr_duplicate()
{
    return new ClientStr(*this);
}

bool
ClientStr::read(const string &)
{
  throw InternalErr(__FILE__, __LINE__, "Called unimplemented read method");
}

void 
ClientStr::print_val(ostream &os, string, bool print_decl_p)
{
    bool print_as_float = false;
    double val = 0.0;

    // Translate all string variables to Floats. jhrg 1/9/98.
    if (string_to_float) {
	char *ptr = NULL;
	const char *str = _buf.c_str();
	val = strtod(str, &ptr);
	print_as_float = !(val == 0.0 && (ptr == str));

	if (!print_as_float) {
	    val = MakeNaN();
	    print_as_float = true;
	    if (warning) {
		cerr << "Could not translate `" << _buf << "' to a Float64,"
		     << endl;
		cerr << "interning as NaN (not a number: " << val 
		     << ")" << endl;
	    }
	}
    }

    if (print_as_float) {
	if (print_decl_p)
	    os << "Float64" << endl << get_matlab_name() << endl;

	if (ascii)
	    os << val << " ";
	else
	    os.write((char *)&val, sizeof(double));
    }
    else {
	if (print_decl_p)
	    os << type_name() << endl << get_matlab_name() 
		<< endl;

	os << _buf << endl;
    }
}

AttrTable &
ClientStr::getAttrTable()
{
    return _attr;
}

void 
ClientStr::setAttrTable(AttrTable &attr)
{
    _attr = attr;
}

void
ClientStr::set_name(const string &n)
{
    BaseType::set_name(n);
    set_matlab_name(n);
}

string 
ClientStr::get_matlab_name() const
{
    return _matlabName; 
}

void 
ClientStr::set_matlab_name(const string &name)
{ 
    _matlabName = names.lookup(name, translate);
}

// $Log: ClientStr.cc,v $
// Revision 1.3  2003/12/08 17:59:49  edavis
// Merge release-3-4 into trunk
//
// Revision 1.1.1.1  2003/10/22 19:43:19  dan
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
// Revision 1.29  2003/05/02 17:16:17  jimg
// I replaced the cast is ostream::write that was (void *) to (char *) to
// get this code to compile with gcc 3.2. This change is also needed for
// VC++, so I was able to remove some of the ifdef WIN32 lines. Also, in
// name_map I added a using namespace std; line.
//
// Revision 1.28  2003/01/29 15:43:52  dan
// Resolved conflict on merge, caused by PWEST's removal
// of the SLLIST includes in a previous update on the trunk.
//
// Revision 1.27  2001/08/27 18:06:57  jimg
// Merged release-3-2-5.
//
// Revision 1.26.2.2  2002/09/05 22:26:49  pwest
// Removed includes to SLList and DLList. These are not necessary and no longer
// supported.
//
// Revision 1.26.2.1  2001/08/21 16:49:18  dan
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
// Revision 1.26  2000/10/03 22:24:17  jimg
// Reorganized the CVS Log entries.
// Changed the read() mfunc definitions to match the 3.2 dap++ library.
//
// Revision 1.25  2000/07/21 10:21:56  rmorris
// Merged with win32-mlclient-branch.
//
// Revision 1.24.2.2  2000/07/09 22:26:32  rmorris
// Mod's to increase portability, minimize ifdef's for win32.
//
// Revision 1.24.2.1  2000/06/26 22:54:26  rmorris
// Mods for port to win32.
//
// Revision 1.24  2000/06/07 00:28:31  jimg
// Merged changes from version 3.1.4
//
// Revision 1.22.2.3  2000/06/02 23:03:32  jimg
// Fixes for AttrTables with no entries.
// Added support for name translation and a DODS_ML_Real_Name attribute.
// Changed the `size' attribtue's name to DODS_ML_Size.
//
// Revision 1.22.2.2  2000/05/26 22:15:54  jimg
// Removed the old DMS time processing code.
//
// Revision 1.23  2000/04/20 23:38:12  jimg
// Merged with release 3.1.3
//
// Revision 1.22.2.1  2000/04/11 21:46:21  jimg
// Removed old code.
//
// Revision 1.22  1999/07/24 00:10:28  jimg
// Merged the release-3-0-2 branch
//
// Revision 1.21  1999/04/30 17:06:55  jimg
// Merged with no-gnu and release-2-24
//
// Revision 1.20  1999/03/24 06:23:43  brent
// convert String.h to std lib <string>, convert to packages regex -- B^2
//
// Revision 1.19  1998/11/25 01:16:19  jimg
// Now processes -w for warnings. This means that verbose (-v) mode is
// information and -w is for warnings only.
//
// Revision 1.18  1998/11/20 08:41:22  jimg
// Fixed the fix for the extra newline separators; it was broken for arrays
//
// Revision 1.17  1998/11/19 20:53:33  jimg
// Fixed the extra newline bug.
//
// Revision 1.16  1998/09/16 23:02:22  jimg
// Removed write_val() and replaced it with print_val() and print_all_vals()
//
// Revision 1.15  1998/08/03 16:39:50  jimg
// Fixed write_val mfunc so that outermost is no longer used
//
// Revision 1.14  1998/06/09 04:09:13  jimg
// Made some of the diagnostic output about interning NaNs part of the verbose
// option. It will only print when writeval is called with the -v option.
//
// Revision 1.13  1998/02/05 20:14:50  jimg
// DODS now compiles with gcc 2.8.x
//
// Revision 1.12  1997/12/02 20:34:37  jimg
// Fixed/Removed the automatic conversion of strings to numbers. This was a
// bad idea.
//
// Revision 1.11  1997/10/04 00:40:06  jimg
// Added changes to support the new deserialize() mfunc and write_val().
//
// Revision 1.10  1997/06/06 04:02:07  jimg
// Added ASCII output option to writeval. Uses a global `flag'.
//
// Revision 1.9  1997/04/19 00:54:58  jimg
// Changed ouput stream to simplify reading (for loaddods).
//
// Revision 1.8  1997/02/06 19:42:39  jimg
// When converting strings to Float64 values, any string that cannot be
// converted is externalized as NaN.
//
// Revision 1.7  1997/01/13 18:01:50  jimg
// Fixed a bug when the degree part of a DDDMMSSH string has only one or two
// digits.
//
// Revision 1.6  1997/01/10 06:49:37  jimg
// Changed call to name_map::lookup() so that non-alphanumerics are mapped to
// underscore. 
//
// Revision 1.5  1996/11/23 05:12:13  jimg
// Added support for variable renaming via the name_map object.
//
// Revision 1.4  1996/11/13 18:02:54  jimg
// Fixed bad usage of strtod(). When strtod() returns 0.0, the pointer passed
// as a second argument must be compared to the pointer to the string to be
// converted instead of testing only that it is not null. If the pointer is not
// equal to the string argument then the number *has* converted successfully
// (to 0.0). Otherwise, the string did not convert.
//
// Revision 1.3  1996/10/23 23:46:44  jimg
// Modified so that writeval outputs a `recursive' data stream. Thus any
// data type can be written and later read without being flattened. This
// change was made so that the NSCAT data which is represented as a Sequence
// of Structures.
//
// Revision 1.2  1996/10/07 21:00:29  jimg
// Fixed Headers.
//
// Revision 1.1  1996/09/30 23:59:18  jimg
// Added.
