
// -*- C++ -*-

// (c) COPYRIGHT URI/MIT 1996
// Please read the full copyright statement in the file COPYRIGH.  
//
// Authors:
//	jhrg,jimg	James Gallagher (jgallagher@gso.uri.edu)

// $Log: name_map.h,v $
// Revision 1.1  2003/10/22 19:43:33  dan
// Initial revision
//
// Revision 1.13  2003/05/02 17:16:17  jimg
// I replaced the cast is ostream::write that was (void *) to (char *) to
// get this code to compile with gcc 3.2. This change is also needed for
// VC++, so I was able to remove some of the ifdef WIN32 lines. Also, in
// name_map I added a using namespace std; line.
//
// Revision 1.12  2000/08/30 00:13:29  jimg
// Merged with 3.1.7
//
// Revision 1.8.2.3  2000/08/25 23:33:23  jimg
// Removed unneeded parts from name_map.
// Added a lookup(string) method (only performs the thesaurus lookup) and
// rewrite the lookup(string, bool) method to use lookup(string).
//
// Revision 1.11  2000/07/21 10:21:56  rmorris
// Merged with win32-mlclient-branch.
//
// Revision 1.9.2.1  2000/06/26 23:02:21  rmorris
// Modification for port to win32.
//
// Revision 1.10  2000/06/20 21:40:09  jimg
// Merged with 3.1.6
//
// Revision 1.8.2.2  2000/06/20 20:36:46  jimg
// Imporved error messages and fixed some doc++ comments (although some
// problems remain).
//
// Revision 1.9  2000/06/07 00:28:32  jimg
// Merged changes from version 3.1.4
//
// Revision 1.8.2.1  2000/06/02 23:03:33  jimg
// Fixes for AttrTables with no entries.
// Added support for name translation and a DODS_ML_Real_Name attribute.
// Changed the `size' attribtue's name to DODS_ML_Size.
//
// Revision 1.8  1999/07/24 00:10:07  jimg
// Repaired the munge function and removed SLList.
//
// Revision 1.7  1999/04/30 17:06:58  jimg
// Merged with no-gnu and release-2-24
//
// Revision 1.6  1999/03/24 06:40:48  brent
// convert String.h to std lib <string>, convert to packages regex -- B^2
//
// Revision 1.5.18.1  1999/03/30 20:26:02  jimg
// Added include of SLList
//
// Revision 1.5  1998/03/13 21:24:26  jimg
// Fixed up comments
//
// Revision 1.4  1997/02/06 20:44:52  jimg
// Fixed log messages
//
// Revision 1.3  1997/01/10 06:50:32  jimg
// Added to lookup mfunc the ability to map non-alphanmerics to underscore.
//
// Revision 1.2  1996/11/23 05:17:56  jimg
// Added name_map().
//
// Revision 1.1  1996/11/22 23:50:21  jimg
// Created.
//

#ifndef _NAME_MAP_H
#define _NAME_MAP_H

#ifdef __GNUG__
#pragma interface
#endif

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
