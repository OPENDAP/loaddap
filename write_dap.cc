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

// write_dap is a simple program that reads DODS datasets and writes the
// binary data to stdout using a simple scheme. The program expects one or
// more URLs will be given as arguments. Each URL will be dereferenced in the
// order given on the command line. Data is written to stdout prefixed by data
// type and size information.

#ifdef WIN32
#include <windows.h>
#endif

#include "config_writedap.h"

static char rcsid[] not_used = {"$Id$"};

#include <iostream>
#include <fstream>
#include <iomanip>
#include <string>
#include <typeinfo>
#include <cstdio>

#ifndef _MSC_VER
#include <GetOpt.h>
#else
#include <GNU/GetOpt.h>
#endif

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

const char *usage_msg =
"\n\
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

static void usage() {
	cout << usage_msg << endl;
}

// Not that smart... Print a new line after writing one of the simple data
// types. This is called by the ctor's write_val() mfuncs so that individual
// values are terminated properly while individual values in an array are not
// terminated.

void smart_newline(FILE *os, Type type) {
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

void smart_newline(ostream &os, Type type) {
	switch (type) {
	case dods_byte_c:
	case dods_int16_c:
	case dods_uint16_c:
	case dods_int32_c:
	case dods_uint32_c:
	case dods_float32_c:
	case dods_float64_c:
	case dods_array_c:
		os.write("\n", 1);
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

	os << flush;
}

static void process_data(Connect &url, DDS *dds) {
	if (verbose)
		cerr << "Server version: " << url.get_version() << endl;

	DBG(cerr << "process_data(): translate:" << translate << ";" << endl);

	DDS::Vars_iter q;
	for (q = dds->var_begin(); q != dds->var_end(); ++q) {
#if 1
	    // The calls using stdout can be replaced with ones that use cout
	    // if the ClientArray::print_val(), ..., methods are overloaded
	    // with versions that take ostream & objects. NB: This is the
	    // last code that uses the FILE* methods, so removing this could
	    // be important for a libdap refactor
	    (*q)->print_val(stdout);
	    smart_newline(stdout, (*q)->type());
#else
	    (*q)->print_val(cout);
	    smart_newline(cout, (*q)->type());
#endif
	}
}

static void process_per_url_options(int &i, int /* argc*/, char *argv[]) {
	names.delete_all(); // Clear the global name map for this URL.

	// Test for per-url option. Set variables accordingly.
	while (argv[i+1] && argv[i+1][0] == '-')
		switch (argv[++i][1]) {
		case 'r':
			++i; // Move past option to argument.
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
 write anything out (since that will confuse loaddap).

 @author jhrg */
static void output_error_object(Error &e) {
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
main(int argc, char * argv[]) {
	GetOpt getopt(argc, argv, "VvwnfFat:AD");
	int option_char;
	bool show_dds = false;
	bool show_das = false;
	string expr = "";
	// Connect dummy("http");

#ifdef WIN32
	_setmode(_fileno(stdout), _O_BINARY);
#endif

	putenv("_POSIX_OPTION_ORDER=1"); // Suppress GetOpt's argv[] permutation.

	while ((option_char = getopt()) != EOF)
		switch (option_char) {
		// General options
		case 'V': {
			cerr << "writedap: " << VERSION << endl;
			exit(0);
		}
		case 'v':
			verbose = true;
			break;
		case 'w':
			warning = true;
			break;
		case 'n':
			translate = true;
			break;
		case 'f':
			numeric_to_float = true;
			break;
		case 'F':
			string_to_float = true;
			break;
		case 'a':
			ascii = true;
			break;
			// Metadata options
		case 'A':
			show_das = true;
			break;
		case 'D':
			show_dds = true;
			break;
			// Help!
		case 'h':
		case '?':
		default:
			usage();
			exit(1);
			break;
		}

	Connect *url = 0;

	// If after processing all the command line options there is nothing left
	// (no URL or file) assume that we should read from stdin. This test
	// allows `-r' options to be used when reading from a pipe or
	// redirection. It is assumed that all command options will be removed by
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

		DBG2(cerr << "argv[" << i << "] (of " << argc << "): " << argv[i]
				<< endl);

		if (url->is_local()) {
			if (verbose)
				cerr << "Assuming that the argument " << argv[i]
						<< " is a file" << endl
						<< "that contains a DODS data object; decoding."
						<< endl;

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
				dds.print(cout);
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
