/*
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
*/

/*
  C versions of the C++ defines used for debugging, etc.
*/

#ifndef __defines_h
#define __defines_h

#ifndef DBG
#ifdef DODS_DEBUG
#define DBG(x) x
#else
#define DBG(x)
#endif
#endif /* DBG */

#ifndef DBG2
#ifdef DODS_DEBUG2
#define DBG2(x) x
#else
#define DBG2(x)
#endif
#endif /* DBG2 */

#ifndef DBGM
#ifdef DODS_DEBUGM
#define DBGM(x) x
#else
#define DBGM(x)
#endif
#endif /* DBGM */

#ifndef TRUE
#define TRUE (1)
#define FALSE (0)
#endif /* TRUE */

#ifndef MAX
#define MAX(x,y) ((x > y) ? (x) : (y))
#endif /* MAX */

#ifdef NO_MX_FREE
#undef mxFree
#define mxFree(x) /* x */
#endif /* NO_MX_FREE */

#endif /* __defines_h */
