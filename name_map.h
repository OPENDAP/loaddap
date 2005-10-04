
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

#ifndef _NAME_MAP_H
#define _NAME_MAP_H

#include <vector>
#include <string>

using namespace std;

/** This class can be used to build up a simple thesaurus which maps names
    from one string to another. The thesaurus is built by calling the #add#
    mfunc with a string of the form #<source>:<dest># where #<source># and
    #<dest># are string literals. Once the thesaurus is built, the #lookup#
    mfunc can be used to get the equivalent of any string in the thesaurus.

    As an additional feature, this class can canonicalize an identifier
    (remove non-alphanumeric characters) when performing the lookup
    operation. This is performed as a separate step from the thesaurus scan
    so words with no entry in the thesaurus will still be canonicalized.

    You can interleave calls to #add# and calls to #lookup#. 

    @memo Map names using a simple thesaurus.
    @author jhrg */

class name_map {
private:
    struct name_equiv {
      // int colon;
	string from;
	string to;
	name_equiv(char *raw_equiv) {
	    string t = raw_equiv;
	    int colon = t.find(":");
	    from = t.substr(0,colon);
	    to = t.substr(colon+1, t.size());
	}
#if 0
	name_equiv() {
	}
	~name_equiv() {
	}
#endif
    };

    vector<name_equiv> _names;
    typedef vector<name_equiv>::const_iterator NEItor;

public:
    /** Create a new instance of the thesaurus and add the first equivalence
        string to it. 
	
	@memo Create a new instance of the thesaurus. */
    name_map(char *raw_equiv);

    /** Create an empty thesaurus. */
    name_map();

    /** Add a new entry to the thesaurus. The form of the new entry is
        <source>:<dest>. */
    void add(char *raw_equiv);

    /** Lookup a word in the thesaurus. If the name appears in the thesaurus,
        return its equivalent. 

	@param name Look this name up.
	@return The string #name# or its thesaurus entry. */
    string lookup(string name);

    /** Lookup a word in the thesaurus. If the name appears in the thesaurus,
        return its equivalent. If #canonical_names# is true, pass the string
	#name# or its equivalent through a filter to replace all characters
	that are not alphanumerics with the underscore. Sequences matching
	`%[0-9][0-9a-fA-F]' are considered to be hex escape codes and are
	replaced by a single underscore.

	NB: This method is here for backward compatibilty with an eariler
	verion of the name_map class.

	@param name Look this name up.
	@param canonical_names If true, return a Matlab sanitized name.
	@return The string #name#, its equivalent or its canonicalized
	equivalent. */
    string lookup(string name, const bool canonical_names);

    /** Delete all the entries from the thesaurus. */
    void delete_all();
};

#endif // _NAME_MAP_H
