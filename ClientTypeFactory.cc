
// -*- mode: c++; c-basic-offset:4 -*-

// This file is part of nc_handler, a data handler for the OPeNDAP data
// server. 

// Copyright (c) 2002,2003 OPeNDAP, Inc.
// Author: James Gallagher <jgallagher@opendap.org>
//
// This is free software; you can redistribute it and/or modify it under the
// terms of the GNU Lesser General Public License as published by the Free
// Software Foundation; either version 2.1 of the License, or (at your
// option) any later version.
// 
// This software is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
// or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public
// License for more details.
// 
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
// You can contact OPeNDAP, Inc. at PO Box 112, Saunderstown, RI. 02874-0112.
 
#include <string>

#include "ClientByte.h"
#include "ClientInt16.h"
#include "ClientUInt16.h"
#include "ClientInt32.h"
#include "ClientUInt32.h"
#include "ClientFloat32.h"
#include "ClientFloat64.h"
#include "ClientStr.h"
#include "ClientUrl.h"
#include "ClientArray.h"
#include "ClientStructure.h"
#include "ClientSequence.h"
#include "ClientGrid.h"

#include "ClientTypeFactory.h"
#include "debug.h"

Byte *
ClientTypeFactory::NewByte(const string &n ) const 
{ 
    return new ClientByte(n);
}

Int16 *
ClientTypeFactory::NewInt16(const string &n ) const 
{ 
    return new ClientInt16(n); 
}

UInt16 *
ClientTypeFactory::NewUInt16(const string &n ) const 
{ 
    return new ClientUInt16(n);
}

Int32 *
ClientTypeFactory::NewInt32(const string &n ) const 
{ 
    DBG(cerr << "Inside ClientTypeFactory::NewInt32" << endl);
    return new ClientInt32(n);
}

UInt32 *
ClientTypeFactory::NewUInt32(const string &n ) const 
{ 
    return new ClientUInt32(n);
}

Float32 *
ClientTypeFactory::NewFloat32(const string &n ) const 
{ 
    return new ClientFloat32(n);
}

Float64 *
ClientTypeFactory::NewFloat64(const string &n ) const 
{ 
    return new ClientFloat64(n);
}

Str *
ClientTypeFactory::NewStr(const string &n ) const 
{ 
    return new ClientStr(n);
}

Url *
ClientTypeFactory::NewUrl(const string &n ) const 
{ 
    return new ClientUrl(n);
}

Array *
ClientTypeFactory::NewArray(const string &n , BaseType *v) const 
{ 
    return new ClientArray(n, v);
}

Structure *
ClientTypeFactory::NewStructure(const string &n ) const 
{ 
    return new ClientStructure(n);
}

Sequence *
ClientTypeFactory::NewSequence(const string &n ) const 
{ 
    DBG(cerr << "Inside ClientTypeFactory::NewSequence" << endl);
    return new ClientSequence(n);
}

Grid *
ClientTypeFactory::NewGrid(const string &n ) const 
{ 
    return new ClientGrid(n);
}
