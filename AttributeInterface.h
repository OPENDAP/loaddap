
// -*- C++ -*-

#ifndef __attribtue_interface_h
#define __attribtue_interface_h

#include "AttrTable.h"
/** This class supplies an interface which children of BaseType can use to
    access an AttrTable instance. The child classes must supply the AttrTable
    instance. 

    @see ClientByte
    @see ClientArray
    @author jhrg */
class AttributeInterface {
 public:
    /** Access an attribute table (an instance of AttrTable)
	@return AttrTable \& */
    virtual AttrTable &getAttrTable() = 0;

    /** Store an attribute table (AttrTable) in the object. 
	@param attr A reference to the attribute table (AttrTable) to store
	in the object. */
    virtual void setAttrTable(AttrTable &attr) = 0;
};

#endif // __attribtue_interface_h
