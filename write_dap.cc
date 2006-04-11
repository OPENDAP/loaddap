
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

// (c) COPYRIGHT URI/MIT 1996,1997,2000
// Please read the full copyright statement in the file COPYRIGHT.
//
// Authors:
//	jhrg,jimg	James Gallagher (jgallagher@gso.uri.edu)

// writeval is a simple program that reads DODS datasets and writes the
// binary data to stdout using a simple scheme. The program expects one or
// more URLs will be given as arguments. Each URL will be dereferenced in the
// order given on the command line. Data is writen to stdout prefixed by data
// type and size information.

#ifdef WIN32
#include <windows.h>
#endif

#include "config_writedap.h"

static char rcsid[] not_used = {"$Id$"};

#include <stdio.h>

#include <string>
#include <typeinfo>

#include <GetOpt.h>

#include <BaseType.h>
#include <Connect.h>
#include <Response.h>
#include <StdinResponse.h>

#include "LoaddodsProcessing.h"
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

#include "name_map.h"
#include "debug.h"

#ifdef WIN32
#include <iostream>
#include <fstream>
#include <fcntl.h>
#include <stdlib.h>
#include <io.h>
#endif

using namespace std;

name_map names;
bool ascii = false;
bool translate = false;
bool numeric_to_float = false;
bool string_to_float = false;
bool verbose = false;
bool warning = false;

#ifndef VERSION
const char *VERSION = "unknown";
#endif /* VERSION */

const char *usage_msg = "\n\
Usage:\n\
Read from a URL: writedap [VvwfFgADat] -- [<url> [-c <expr>] [[-r <var>:<newvar>] ...] ...]\n\
Read from stdin: writedap [VvwfFga] -- - [-r <var>:<newvar>]\n\
\n\
General options:\n\
      V: Print the version number of this program and exit.\n\
      v: Verbose output.\n\
      w: Show warnings.\n\
      n: Turn on name canonicalization. This maps WWW hex codes to\n\
         underscores.\n\
      f: Force all numeric types to float.\n\
      F: Force all string variables to be translated to Float64.\n\
      a: ASCII output. This is intended mostly for debugging.\n\
      t: Trace network I/O (HTTP, DNS, ...). See geturl\n\
\n\
      A: Combine dataset structure and attribute information and\n\
         return it so that a structured representation of the dataset\n\
         can be built.\n\
      D: Print the DDS to standard ouput. This does not include DAS\n\
         information.\n\
\n\
Per-URL options:\n\
      c: Per-URL constraint expression, enclosed in quotes.\n\
      r: Per-URL name mappings; var becomes newvar.\n\
\n\
Notes:\n\
\n\
By default writedap dereferences the URL and writes the data\n\
in binary on stdout. Use the -D option to see the dataset's\n\
DDS, and -A to request structured attribute information.\n\
\n\
Note that -D, -A, -r and -c work only when writedap is given\n\
one or more URLs.";

static void
usage()
{
    cout << usage_msg << endl;
}

#if 0
// Not used. jhrg 2/10/06
static string
name_from_url(string url)
{
    // find the last part of the URL (after the last `/') and then strip off
    // the trailing extension (anything following the `.')
    int start = url.rfind("/") + 1;
    int end = url.rfind(".");
    
    string name = url.substr(start, end-start);

    return name;
}
#endif

// Not that smart... Print a new line after writing one of the simple data
// types. This is called by the ctor's write_val() mfuncs so that individual
// values are terminated properly while individual values in an array are not
// terminated.

void
smart_newline(FILE *os, Type type)
{
    switch (type) {
      case dods_byte_c:
      case dods_int16_c:
      case dods_uint16_c:
      case dods_int32_c:
      case dods_uint32_c:
      case dods_float32_c:
      case dods_float64_c:
      case dods_array_c:
	fprintf(os, "\n");
	break;

	// strings are lumped in with the ctors because strings end in a
	// new line character and therefore don't need one appended.
      case dods_str_c:
      case dods_url_c:
      case dods_sequence_c:
      case dods_structure_c:
      case dods_grid_c:
      default:
	break;
    }
    
    fflush(os);
}

static void
process_data(Connect &url, DDS *dds)
{
    if (verbose)
	cerr << "Server version: " << url.get_version() << endl;
    
    DBG(cerr << "process_data(): translate:" << translate << ";" << endl);

    DDS::Vars_iter q;
    for (q = dds->var_begin(); q != dds->var_end(); ++q) {
	(*q)->print_val(stdout);
	smart_newline(stdout, (*q)->type());
    }
}

static void
process_per_url_options(int &i, int /* argc*/, char *argv[])
{
    names.delete_all();	// Clear the global name map for this URL.

    // Test for per-url option. Set variables accordingly.
    while (argv[i+1] && argv[i+1][0] == '-')
	switch (argv[++i][1]) {
	  case 'r':
	    ++i;	// Move past option to argument.
	    if (verbose)
		cerr << "  Renaming: " << argv[i] << endl;
	    // Note that NAMES is a global variable so that all the
	    // writedap() mfuncs can access it without having to pass
	    // it into each function call.
	    names.add(argv[i]);
	    break;
		
	  case 'c':
	    ++i;
	    if (verbose)
		cerr << "  Constraint: " << argv[i] << " ignored." << endl;
	    break;
		
	  default:
	    cerr << "Unknown option `" << argv[i][1] 
		 << "' paired with URL has been ignored." << endl;
	    break;
	}

}

/** Write out the given error object. If the Error object #e# is empty, don't
    write anything out (since that will confuse loaddods).

    @author jhrg */
static void
output_error_object(Error &e)
{
    if (e.OK()) {
	cout << "Error" << endl;
	cout << e.get_error_message() << endl << endl;
    }
}

#ifdef WIN32
void
#else
int
#endif
main(int argc, char * argv[])
{
    GetOpt getopt (argc, argv, "VvwnfFat:AD");
    int option_char;
    bool show_dds = false;
    bool show_das = false;
    string expr = "";
#if 0
    bool cexpr = false;
    char *tcode = NULL;
    int topts = 0;
#endif
    // Connect dummy("http");

#ifdef WIN32
    _setmode(_fileno(stdout), _O_BINARY);
#endif

    putenv("_POSIX_OPTION_ORDER=1"); // Suppress GetOpt's argv[] permutation.

    while ((option_char = getopt()) != EOF)
	switch (option_char) {
	    // Genreal options
	  case 'V': {cerr << "writedap: " << VERSION << endl; exit(0);}
	  case 'v': verbose = true; break;
	  case 'w': warning = true; break;
	  case 'n': translate = true; break;
	  case 'f': numeric_to_float = true; break;
	  case 'F': string_to_float = true; break;
	  case 'a': ascii = true; break;
	    // Metadata options
	  case 'A': show_das = true; break;
	  case 'D': show_dds = true; break;
	    // Help!
	  case 'h':
	  case '?':
	  default:
	    usage(); exit(1); break;
	}

    Connect *url = 0;

    // If after processing all the command line options there is nothing left
    // (no URL or file) assume that we should read from stdin. This test
    // allows `-r' options to be used when reading from a pipe or
    // redirection. It is assumed that all command options will be rmoved by
    // the time execution gets here. Thus, if an option appears here we must
    // be reading from stdin.
    if (getopt.optind == argc || argv[getopt.optind][0] == '-') {
	if (verbose)
	    cerr << "Assuming standard input is a DODS data stream." << endl;

	try {
	    url = new Connect("stdin");

	    process_per_url_options(getopt.optind, argc, argv);

	    ClientTypeFactory factory;
	    DataDDS data(&factory);
            StdinResponse r(stdin);
	    url->read_data(data, &r);

	    process_data(*url, &data);
	}
	catch (Error &e) {
	    output_error_object(e);
	}

	exit(0);
    }

    for (int i = getopt.optind; i < argc; ++i) {
	if (url)
	    delete url;

	url = new Connect(argv[i]);

	DBG2(cerr << "argv[" << i << "] (of " << argc << "): " << argv[i] \
	     << endl);

	if (url->is_local()) {
	    if (verbose) 
		cerr << "Assuming that the argument " << argv[i] 
		     << " is a file" << endl 
		     << "that contains a DODS data object; decoding." << endl;

	    process_per_url_options(i, argc, argv);

	    try {
		ClientTypeFactory factory;
		DataDDS data(&factory);
                Response r(fopen(argv[i], "r"));
		url->read_data(data, &r);
		process_data(*url, &data);
	    }
	    catch (Error &e) {
		output_error_object(e);
	    }	      

	    continue;
	}

	if (verbose)
	    cerr << endl << "Reading: " << url->URL(false) << endl;
	
	// If the user supplied -d or -D, print the DDS
	if (show_dds) {
	    try {
		ClientTypeFactory factory;
		DDS dds(&factory);
		url->request_dds(dds);
		dds.print(stdout);
	    }
	    catch (Error &e) {
		output_error_object(e);
	    }
	    continue;
	}

	// If the user supplied -A, write the attribute material.
	if (show_das) {
	    try {
		DAS das;
		url->request_das(das);
		ClientTypeFactory factory;
		DDS dds(&factory);
		url->request_dds(dds);
		LoaddodsProcessing lp(dds);
	    
		lp.transfer_attributes(das);
		//lp.prune_duplicates();
		lp.add_size_attributes();
		lp.add_realname_attributes();
		lp.print_for_matlab(cout);
	    }
	    catch (Error &e) {
		output_error_object(e);
	    }
	    continue;
	}

	// If writedap is not reading from a pipe or a local file and is
	// not being used to access the DAS or DDS, the caller must want
	// data. 
	{
	    // Scan ahead for a -c option. If it does not exist, print the CE
	    // here before printing the stuff about renaming variables.
	    // Otherwise delay printing the constraint until the in the
	    // second while-loop.
	    int j = i;
	    bool found_ce = false;
	    while (!found_ce && j < argc && argv[j+1] && argv[j+1][0] == '-')
		if (argv[++j][1] == 'c') {
		    found_ce = true;
		    expr = argv[++j];
		    i = j;
		}

	    if (verbose && !found_ce)
		cerr << "  Constraint: " << url->CE() << endl;

	    process_per_url_options(i, argc, argv);

	    try {
		ClientTypeFactory factory;
		DataDDS dds(&factory);
		url->request_data(dds, expr);

		DBG(cerr << "Starting writing data..." << endl);

		process_data(*url, &dds);
	    }
	    catch (Error &e) {
		output_error_object(e);
	    }
	    DBG(cerr << "Completed writing data." << endl);
	}

	cout.flush();
    } // end of the URL processing loop

    DBG(cerr << "writedap exiting." << endl);
    cerr.flush();

    delete url;

#ifdef WIN32
    exit(0);
    return;
#else
    exit(0);
#endif
}

// $Log: write_dap.cc,v $
// Revision 1.4  2004/07/08 20:50:03  jimg
// Merged release-3-4-5FCS
//
// Revision 1.1.2.2  2004/05/04 15:48:35  dan
// Removed call to LoaddodsProcessing::prune_duplicates() when processing
// the DAS response.  This prunes the duplicated map vectors that the
// netCDF server prefixes to its response for Grid type variables.   The
// problem is that this pruning is only occuring for DAS handling and
// has led to confusion by users.
//
// Revision 1.3  2004/01/26 17:38:57  jimg
// Removed List from the DAP
//
// Revision 1.2  2003/12/08 17:59:50  edavis
// Merge release-3-4 into trunk
//
// Revision 1.1.2.1  2003/10/27 16:59:42  dan
// Modified versions of 'loaddods' and 'writeval' using
// new naming conventions for 'loaddap' and 'writedap', respectively.
//
// Revision 1.1.1.1  2003/10/22 19:43:36  dan
// Version of the Matlab CommandLine client which uses Matlab Structure
// variables to maintain the shape of the underlying DODS data.
//
// Revision 1.44  2003/03/06 18:49:28  jimg
// Replaced call to Connect::server_version() with Connect::get_version(). The
// server_vesion() method no longer exists.
//
// Revision 1.43  2003/01/29 15:43:52  dan
// Resolved conflict on merge, caused by PWEST's removal
// of the SLLIST includes in a previous update on the trunk.
//
// Revision 1.42  2001/09/29 00:08:01  jimg
// Merged with 3.2.6.
//
// Revision 1.40.2.10  2002/09/08 23:55:53  rmorris
// Include io.h for win32
//
// Revision 1.40.2.9  2002/09/05 22:26:49  pwest
// Removed includes to SLList and DLList. These are not necessary and no longer
// supported.
//
// Revision 1.40.2.8  2002/06/21 00:31:39  jimg
// I changed many files throughout the source so that the 'make World' build
// works with the new versions of Connect and libdap++ that use libcurl.
// Most of these changes are either to Makefiles, configure scripts or to
// the headers included by various C++ files. In a few places the oddities
// of libwww forced us to hack up code and I've undone those and some of the
// clients had code that supported libwww's generous tracing capabilities
// (that's one part of libwww I'll miss); I had to remove support for that.
// Once this code compiles and more work is done on Connect, I'll return to
// each of these changes and polish them.
//
// Revision 1.40.2.7  2002/04/22 23:08:12  jimg
// I've added support for the progress gui back into the code. I'm not sure why
// it was removed. If loaddods, et c. should not have the GUI pop up, then it
// should not use the -g option when it calls writeval.
//
// Revision 1.40.2.6  2002/04/22 21:51:02  jimg
// Corrected the help message.
//
// Revision 1.40.2.5  2002/01/16 00:32:32  jimg
// A while ago I changed the way Sequences are deserialized in the DAP. To
// accommodate these changes the code in ClientSequence, ClientStructure and
// process_data had to be fixed.
// NB: It would be better to not use BaseType::print_val(...) but instead
// use a mixin for writing out writeval's stuff. See the asciival code for
// an example.
//
// Revision 1.40.2.4  2001/09/27 15:50:06  jimg
// Added debug.h
//
// Revision 1.41  2001/08/27 18:06:57  jimg
// Merged release-3-2-5.
//
// Revision 1.40.2.3  2001/08/21 16:52:31  dan
// Added ClientFloat32 include file.
//
// Revision 1.40.2.2  2001/06/20 02:47:24  jimg
// Changed the test on line 357 (see note).
//
// Revision 1.40.2.1  2001/05/15 18:28:38  jimg
// Changed the return 0 line (for normal exits) to exit(0). This is part of an
// attempt to work around problems loaddods. That program does not always
// catch the exit status of write_val.
//
// Revision 1.40  2000/10/30 17:27:57  jimg
// Fixes for exception-based error reporting.
//
// Revision 1.39  2000/10/03 22:24:17  jimg
// Reorganized the CVS Log entries.
// Changed the read() mfunc definitions to match the 3.2 dap++ library.
//
// Revision 1.38  2000/08/30 00:13:29  jimg
// Merged with 3.1.7
//
// Revision 1.34.2.6  2000/08/25 23:31:06  jimg
// Added a -n option to writeval. This causes name mapping to be turned on.
// The -f option causes all numerical types to be output as Float64. Before
// -f did both the what the current -f and -n options now do. This makes
// things simpler to understand in this code.
//
// Revision 1.34.2.5  2000/08/17 23:53:53  jimg
// Switched from config_dap.h to config_writedap.h.
//
// Revision 1.37  2000/07/21 10:21:56  rmorris
// Merged with win32-mlclient-branch.
//
// Revision 1.36.2.4  2000/07/19 22:52:37  rmorris
// Call and return from main appropriately for Visual C++ under win32.
//
// Revision 1.36.2.3  2000/07/13 07:12:50  rmorris
// Set stdout in binary mode for win32 and delete the Connect object so
// that the core's intermediate (temporary) file goes away in the win32 case
// after processing is done.
//
// Revision 1.36.2.2  2000/07/09 22:26:33  rmorris
// Mod's to increase portability, minimize ifdef's for win32.
//
// Revision 1.36.2.1  2000/06/26 23:02:21  rmorris
// Modification for port to win32.
//
// Revision 1.36  2000/06/07 00:28:32  jimg
// Merged changes from version 3.1.4
//
// Revision 1.34.2.4  2000/06/02 23:03:33  jimg
// Fixes for AttrTables with no entries.
// Added support for name translation and a DODS_ML_Real_Name attribute.
// Changed the `size' attribtue's name to DODS_ML_Size.
//
// Revision 1.34.2.3  2000/05/26 22:24:44  jimg
// Removed old options and related variables.
// Added new code to use the LoaddodsProcessing object.
//
// Revision 1.34.2.2  2000/05/12 19:15:31  jimg
// This is a non-releaseable version that uses the new MetadataProcessing
// code.
//
// Revision 1.34.2.1  1999/11/02 23:49:38  jimg
// Added instrumentation
//
// Revision 1.34  1999/07/24 00:10:29  jimg
// Merged the release-3-0-2 branch
//
// Revision 1.33  1999/04/30 17:06:58  jimg
// Merged with no-gnu and release-2-24
//
// Revision 1.32  1999/03/24 06:23:43  brent
// convert String.h to std lib <string>, convert to packages regex -- B^2
//
// Revision 1.31.8.1  1999/04/29 18:49:55  jimg
// Fixed version so that it comes from the version.h file (passed in via the
// Makefile.
//
// Revision 1.31  1998/11/25 01:16:19  jimg
// Now processes -w for warnings. This means that verbose (-v) mode is
// information and -w is for warnings only.
//
// Revision 1.30  1998/11/20 08:41:22  jimg
// Fixed the fix for the extra newline separators; it was broken for arrays
//
// Revision 1.29  1998/11/19 20:54:54  jimg
// Fixed the extra newline bug.
//
// Revision 1.28  1998/11/10 03:18:55  jimg
// moved the version number up one notch
//
// Revision 1.27  1998/09/16 23:29:29  jimg
// Fixed the call to ClientStructure::print_all_vals().
//
// Revision 1.26  1998/09/16 23:02:45  jimg
// Removed write_val() and replaced it with print_val() and print_all_vals()
//
// Revision 1.25  1998/08/03 16:39:55  jimg
// Fixed write_val mfunc so that outermost is no longer used
//
// Revision 1.24  1998/06/09 04:10:03  jimg
// Verbose is now a global flag.
//
// Revision 1.23  1998/06/04 06:33:16  jimg
// writeval now prints any message from the Connect's error object if the
// returned dds is null (a flag used to indicate read errors). This means that
// `host not found' errors will be reported to loaddods, etc.
//
// Revision 1.22  1998/03/19 23:27:18  jimg
// Added code to write out error messages read from the server. The format of
// this output is described in the documentation for write_val.
//
// Revision 1.21  1998/02/05 20:14:54  jimg
// DODS now compiles with gcc 2.8.x
//
// Revision 1.20  1997/10/04 00:40:14  jimg
// Added changes to support the new deserialize() mfunc and write_val().
//
// Revision 1.19  1997/09/10 22:51:05  jimg
// Added a `print das' option (-b). This prints the DAS object to stdout.
//
// Revision 1.18  1997/09/05 17:19:07  jimg
// The -A option now accepts the -c option. Use -c to `project' one specific
// attribute container from the DAS. This filters out all the other containers
// so that if a given dataset has an attribute container named `NC_GLOBAL' a
// user can request only the attributes in that container. If the named
// container does not exist, then writeval writes nothing to stdout.
//
// Revision 1.17  1997/06/23 18:34:27  jimg
// Fixed up the online help.
// Added a DDS + data option.
// Updated the version number to 1.4.
//
// Revision 1.16  1997/06/09 20:47:07  jimg
// Fixed a bug in process_per_url_options().
//
// Revision 1.15  1997/06/06 04:06:54  jimg
// Massive changes/additions. Now can read the DODS data stream from stdin
// *and* can write ASCII output (Same output `grammar' as with the binary mode
// but the data is encoded as ASCII). Thus writeval can now serve as a
// server-side filter and a client-side post-retrieval processing agent.
// Updated version to 1.3.
// Added a new option -a to trigger the ASCII output.
//
// Revision 1.14  1997/04/21 20:28:46  jimg
// Updated version number to 1.2 - this matches version 2.13a of DODS.
//
// Revision 1.13  1997/04/19 00:55:01  jimg
// Changed ouput stream to simplify reading (for loaddods).
//
// Revision 1.12  1997/04/08 17:28:30  jimg
// Added test of DDS * returned by Connect::request_data(). This fixes the bug
// where the request results in an error and writeval dumps core after printing
// the message.
//
// Revision 1.11  1997/02/11 16:46:38  jimg
// Removed errant `delete dds' after for loop which processes the variables.
// Don't delete that pointer!
//
// Revision 1.10  1997/02/06 20:46:56  jimg
// Changed verbose to trace and introduced a better verbose mode.
//
// Revision 1.9  1997/02/03 21:25:44  jimg
// Added version information.
//
// Revision 1.8  1997/01/10 06:51:22  jimg
// Added to the online documentation
//
// Revision 1.7  1996/12/18 00:54:20  jimg
// Added code to rename variables and process constraint expressions
// separately from URLs.
//
// Revision 1.6  1996/11/23 05:16:51  jimg
// Added support for per-URL variable renaming. One or more variables can be
// renamed and the set of mappings can be different for each URL.
//
// Revision 1.5  1996/11/13 17:58:40  jimg
// Fixed errors in help text.
//
// Revision 1.4  1996/10/25 23:13:06  jimg
// Added smart_newline() function.
//
// Revision 1.3  1996/10/23 23:49:59  jimg
// Added -f option; map Byte and Int32 types to Float64 and map string to
// Float64 when it can be successfully converted (using strtod()).
// Add changes so that the new write_val(...) (which writes recursive data out)
// will be called.
//
// Revision 1.2  1996/10/07 21:13:51  jimg
// Fixed header.
// Removed redundent handling of embedded constraints (that is not handled by
// the Connect class).
//
// Revision 1.1  1996/09/30 23:59:41  jimg
// Added.
