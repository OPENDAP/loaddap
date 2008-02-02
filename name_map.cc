
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
//	jhrg,jimg	James Gallagher (jgallagher@gso.uri.edu)

#include "config_writedap.h"

static char rcsid[] not_used = {"$Id$"};

#include <ctype.h>

#include <vector>
#include <string>

#include <GNURegex.h>

#include "escaping.h"
#include "name_map.h"

using namespace libdap ;

name_map::name_map(char *raw_equiv) 
{
    _names.push_back(name_equiv(raw_equiv));
}

name_map::name_map()
{
}

void 
name_map::add(char *raw_equiv) 
{
    name_equiv equiv(raw_equiv);
    _names.push_back(equiv);
}

/** Scan a string and replace anything other than an alphahumeric character
    with a web-style escape sequence. Then rescan the string and repace the 
    '%' characters in those escape sequences with and underscore so that 
    Matlab won't think the are comments.
    
    @param name The string to modify
    @return a new string */
static string
munge(string name)
{
    unsigned char ascii = *(name.substr(0, 1).data());
    if (!isalpha(ascii))
        name = "ml_" + name;

    name = id2www(name, "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789_");
    
    string::size_type pos;
    while ((pos = name.find('%')) != string::npos)
        name.replace(pos, 1, "_");
    
    return name;
}

string
name_map::lookup(string name, const bool canonical_names) 
{
  string new_name = lookup(name);
    if (!canonical_names)
	return new_name;
    else
	return munge(new_name);
}

string
name_map::lookup(string name) 
{
  // NEItor is the name_eqiv const interator. 7/23/99 jhrg
  for (NEItor p = _names.begin(); p != _names.end(); ++p)
    if (name == p->from)
      return p->to;

  return name;
}

void 
name_map::delete_all() 
{
    _names.erase(_names.begin(), _names.end());
}

#ifdef DODS_NAME_MAP_TEST
int
main(int argc, char *argv[])
{
  name_map *nm = new name_map(); 
  string res = nm->lookup("id.long", true);
  cerr << "id.long" << ": " << res << endl;

  res = nm->lookup("id$long", true);
  cerr << "id$long" << ": " << res << endl;

  res = nm->lookup("id%20long", true);
  cerr << "id%20long" << ": " << res << endl;

  delete nm;
}
#endif

// $Log: name_map.cc,v $
// Revision 1.5  2004/02/06 15:52:12  jimg
// Added ctype.h include.
//
// Revision 1.4  2004/02/06 00:49:59  jimg
// Removed strstream include.
//
// Revision 1.3  2003/12/08 17:59:50  edavis
// Merge release-3-4 into trunk
//
// Revision 1.1.1.1  2003/10/22 19:43:33  dan
// Version of the Matlab CommandLine client which uses Matlab Structure
// variables to maintain the shape of the underlying DODS data.
// Revision 1.1.1.1.2.2  2003/10/29 19:03:21  dan
// Removed 'pragma interface' directive from all subclass
// source files.
//
// Revision 1.1.1.1.2.1  2003/10/27 16:48:59  dan
// Changed config include to 'config_writedap.h' to designate
// new version of 'writeval' now called 'writedap'.  The only
// substantive change in 'writedap' is that nested sequence
// variables are now supported.
//
// Revision 1.2  2003/10/23 18:53:02  dan
// Changed config include to config_writedap.h from config_writeval.h
// This is to remain consistent with the renaming used from loaddods
// to loaddap.  To support nested sequences writeval was modified
// to send an end-of-sequence marker to delimit sequence instances.
//
//
// Revision 1.10  2003/01/29 16:18:14  dan
// Merged with release-3-2-7
//
// Revision 1.9.2.4  2002/08/20 13:39:21  dan
// Added 'ml_' to beginning of variable names
// which need to be 'munged' to conform to the
// Matlab variable naming convention.
//
// Revision 1.9.2.3  2002/01/29 22:41:19  dan
// Removed function char2ASCII, this function is now included
// in the dap/escaping.cc code.
//
// Revision 1.9.2.2  2002/01/29 20:37:28  dan
// Modified char2ASCII to return Ascii characters representing
// the Hex numbers in the escaped string, and not their Ascii
// Decimal representation.
//
// Revision 1.9.2.1  2001/11/01 15:11:12  rmorris
// Added include of assert.h.
//
// Revision 1.9  2000/11/22 23:43:00  jimg
// Merge with pre-release 3.2
//
// Revision 1.6.2.3  2000/09/22 21:16:14  jimg
// Added char2ASCII function. Given a string, replace any characters that
// match the regular expression with `_<ASCII code>'. Changed the munge
// function so that it now calls char2ASCII() and not esc2underscore().
//
// Revision 1.8  2000/08/30 00:13:29  jimg
// Merged with 3.1.7
//
// Revision 1.6.2.2  2000/08/25 23:34:30  jimg
// Added a lookup(string) method that does only the thesaurus lookup and
// rewrite lookup(string, bool) to use it.
//
// Revision 1.6.2.1  2000/08/17 23:53:18  jimg
// Switched from config_dap.h to config_writedap.h.
//
// Revision 1.7  2000/07/21 10:21:56  rmorris
// Merged with win32-mlclient-branch.
//
// Revision 1.6  1999/07/24 00:10:07  jimg
// Repaired the munge function and removed SLList.
//
// Revision 1.5  1999/03/24 06:23:43  brent
// convert String.h to std lib <string>, convert to packages regex -- B^2
//
// Revision 1.4  1997/02/06 20:44:51  jimg
// Fixed log messages
//
// Revision 1.3  1997/01/10 06:50:30  jimg
// Added to lookup mfunc the ability to map non-alphanmerics to underscore.
//
// Revision 1.2  1996/11/23 05:17:39  jimg
// Added name_map() because g++ wants it.
//
// Revision 1.1  1996/11/22 23:52:01  jimg
// Created.


