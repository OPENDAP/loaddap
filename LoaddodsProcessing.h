// -*- C++ -*-

// (c) COPYRIGHT URI 2000
// Please read the full copyright statement in the file COPYRIGHT.
//
// Authors:
//	jhrg,jimg	James Gallagher (jgallagher@gso.uri.edu)

#include "DDS.h"
#include "Error.h"
#include "InternalErr.h"
#include "MetadataProcessing.h"

/** Print out text and binary values which the loaddods ML command extension
    can read and use to build a representation of this dataset. The dataset
    is represented as a ML structure where each field is a variable. For each
    variable, the field's value is a structure which contains all of that
    variable's attribute information. Additionally, if the variable is a
    constructor variable, it will have entries for each of its child
    variables. 

    @author jhrg 
    @see DAS
    @see DDS
    @see MetadataProcessing
    @see writeval */

class LoaddodsProcessing: public MetadataProcessing {
protected:
    /** Print out the attributes of the variable #bt# as a Structure.
	Matlab/loaddods will intern the attribtues in a ML structure. */
    void print_attributes(BaseType &bt, ostream &os);

    /** Print an attribute table. DODS Strings and URLs are externalized as
	strings, all numeric types are externalized as Float64s. */
    void print_attr_table(AttrTable &at, ostream &os);
    
    /** Only let children create empty instances */
    LoaddodsProcessing();

public:
    /** Create an empty MetadataProcessing object. */
    LoaddodsProcessing(DDS &dds);

    /** Remove duplicate variables that are added to the DDS by some servers.
	This method uses ad hoc rules to figure out which variables are
	duplicates. */
    void prune_duplicates();

    /** Add the DODS\_ML\_Size attribute. This is added for Array and Grid
	variables. */
    void add_size_attributes();

    /** Tear through the beast and add a DODS\_Real\_Name attribute for each
	variable iff the flag #translate# is true. This provides the caller
	of loaddods with a string that can be used to build URLs which
	contain variable names that will work over on the server. The
	Translate flag folds various escape sequences (e.g., %20) to
	underscores. This is necessary because Matlab uses the `%' character
	to start comments and thus `%20' in a variable name really confuses
	it! */
    void add_realname_attributes();

    /** Write the hierarchically structured attribute information.
	@param os An output stream object for the DAS/DDS inforamtion. */
    void print_for_matlab(ostream &os);
};
    
// $Log: LoaddodsProcessing.h,v $
// Revision 1.1  2003/10/22 19:43:32  dan
// Initial revision
//
// Revision 1.4  2000/07/21 10:21:56  rmorris
// Merged with win32-mlclient-branch.
//
// Revision 1.2.2.1  2000/06/26 23:03:39  rmorris
// Mods for port to win32.
//
// Revision 1.3  2000/06/20 21:40:08  jimg
// Merged with 3.1.6
//
// Revision 1.1.2.3  2000/06/20 20:36:45  jimg
// Imporved error messages and fixed some doc++ comments (although some
// problems remain).
//
// Revision 1.2  2000/06/07 00:28:31  jimg
// Merged changes from version 3.1.4
//
// Revision 1.1.2.2  2000/06/02 23:03:33  jimg
// Fixes for AttrTables with no entries.
// Added support for name translation and a DODS_ML_Real_Name attribute.
// Changed the `size' attribtue's name to DODS_ML_Size.
//
// Revision 1.1.2.1  2000/05/26 20:42:09  jimg
// Added.
//
