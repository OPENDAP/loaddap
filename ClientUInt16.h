
// -*- C++ -*-

// (c) COPYRIGHT URI/MIT 1999
// Please read the full copyright statement in the file COPYRIGHT.
//
// Authors:
//      jhrg,jimg       James Gallagher (jgallagher@gso.uri.edu)

// ClientUInt16 interface. See ClientByte.h for more info.
//
// 3/29/99 jhrg

#ifndef _clientuint16_h
#define _clientuint16_h 1

#ifdef __GNUG__
#pragma interface
#endif

#include "UInt16.h"
#include "AttrTable.h"
#include "AttributeInterface.h"

class ClientUInt16: public UInt16, public AttributeInterface {
private:
    AttrTable _attr;
    string _matlabName;

public:
    ClientUInt16(const string &n = (char *)0);
    virtual ~ClientUInt16() {}

    virtual BaseType *ptr_duplicate();
    
    virtual bool read(const string &dataset);

    virtual void print_val(ostream &os, string space = "", 
			   bool print_decl_p = true);

    virtual AttrTable &getAttrTable();
    virtual void setAttrTable(AttrTable &attr);

    virtual void set_name(const string &n);

    virtual string get_matlab_name() const;
    virtual void set_matlab_name(const string &n);
};

// $Log: ClientUInt16.h,v $
// Revision 1.1  2003/10/22 19:43:20  dan
// Initial revision
//
// Revision 1.6  2001/08/27 18:06:57  jimg
// Merged release-3-2-5.
//
// Revision 1.5.2.1  2001/08/21 16:43:46  dan
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
// Revision 1.3.2.1  2000/06/26 23:00:12  rmorris
// Change for port to win32.
//
// Revision 1.3  2000/06/07 00:28:31  jimg
// Merged changes from version 3.1.4
//
// Revision 1.2.8.2  2000/06/02 23:03:32  jimg
// Fixes for AttrTables with no entries.
// Added support for name translation and a DODS_ML_Real_Name attribute.
// Changed the `size' attribtue's name to DODS_ML_Size.
//
// Revision 1.2.8.1  2000/05/12 19:14:15  jimg
// Now inherits from AttrTable, too
//
// Revision 1.2  1999/04/30 17:06:55  jimg
// Merged with no-gnu and release-2-24
//
// Revision 1.1  1999/03/29 21:22:55  jimg
// Added
//

#endif

