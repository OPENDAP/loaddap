
This package contains the OPeNDAP Matlab command loaddap and the software library it requires. What follows are the ReadMe documents from each of those packages - both lodddap and libdap are available from www.opendap.org separately.

---------------------------------------



 $Id: README 18874 2008-06-11 22:24:59Z jimg $

Updated for 3.6.1

Fixes for the URLs in the online documentation.

Updated for version 3.6.0

** Linux RPM USERS

If you're reading this after installing using theLinux RPM, loaddap installsinto /usr/bin from the RPM and that's the only directory you need to add toyour matlabpath (because the help files go there as well; we install them ina bin directory because automake wants to put the scripts there and becauseit makes for a simpler configuration in Matlab).

** End Linux RPM Users

Be sure to add loaddap's directory to your matlabpath! For example, you couldadd

     addpath /usr/local/bin

to startup.m. The order of the paths' in the matlabpath variable isimportant! However you add the paths, /usr/local/bin should appearbefore /usr/local/share/loaddap. If the order is reversed you will geta error in matlab about mismatched parentheses. Here's what matlabpathlooks like when it's set correctly:

     >> matlabpath

                MATLABPATH

        /usr/local/bin
        /Applications/MATLAB71/toolbox/matlab/general
	.
	.
	.


Although the documentation says that you must use the -nojvm optionwhen starting Matlab, that is not really true based on the latesttesting. The loaddap command correctly reads and interns values whenyou start Matlab either with or without the -nojvm option. However,the verbose output from loaddap is lost when you do not start Matlabusing -nojvm.

Note: For some of the binary builds you may see a message like:

  Unable to load mex file:
  /home/xcst/codes_programms/cmex/yprime.mexglx.
  /usr/local/matlab7/bin/glnx86/../../sys/os/glnx86/libgcc_s.so.1:
  version `GCC_3.3' not found (required by /usr/lib/libstdc++.so.6)
  ??? Invalid MEX-file '/home/xcst/codes_programms/cmex/yprime.mexglx': 


This happens because some linux distributions ship with newer versions ofthe GNU gcc compiler than the version Mathworks uses to build Matlab. A fix for this (documented on the Mathworks web site) is to rename <matlab root>/sys/os/glnx86/libgcc_s.so.1 to something else such as libgcc_s.so.1.orig. With that change Matlab will now use the libgcc librarybundled with your operating system. It's better to rename the library and notdelete it just in case the version bundled with you OS doesn't work withyour copy of Maltab (in which case you can just undothe rename; you can'tuse loaddap, but at least Matlab will still work).

You may also get this error if you compile loaddap yourself. In that caseyou have two choices. Either use the above fix or get and install the sameversion of GNU's gcc compiler as was used to build your copy of Matlab and use that to build libdap and loaddap.

-------------------------------------------------------------------------------

For installation information, see the file INSTALL.

CONTENTS OF THIS DIRECTORY

The two main programs in this directory are loaddap and whodap.  Theseare Matlab command line clients from OPeNDAP which provide a way toread data from DAP-enabled servers directly into Matlab. The loaddapfunction reads data from an OPeNDAP server and interns it in theMatlab workspace.  The whodap function provides a listing of thevariables in a dataset. Both commands accept DAP URLs as arguments.

whodap <dataset URL>

Example

    >> whodap http://www.opendap.org/opendap/nph-dods/data/fnoc1.nc
    Dataset {
	Int16 u[time_a = 16][lat = 17][lon = 21];
	Int16 v[time_a = 16][lat = 17][lon = 21];
	Float32 lat[lat = 17];
	Float32 lon[lon = 21];
	Float32 time[time = 16];
    } fnoc1;

    loaddap <dataset URL>?<variables>


Example:

    >> loaddap 'http://www.opendap.org/opendap/nph-dods/data/fnoc1.nc?u,v'

    Reading: http://www.opendap.org/opendap/nph-dods/data/fnoc1.nc
      Constraint: u,v
    Server version: nc/3.4.8
    Creating matrix u (16 x 17 x 21) with 5712 elements.
    Creating matrix v (16 x 17 x 21) with 5712 elements.


Note that you need the quotes around the argument to loaddap becauseMatlab recognizes commas as special characters.

Both whodap and loaddap have online help (type `help loaddap' at theMatlab prompt). Loaddap has many options, all described in the on-linehelp. It can accept multiple URLs, rename variables, be used to readdataset attribute information and be used in assignmentstatements. Here's an example of loaddap called with an option andreturning values using an assignment statement. The -e option tellsloaddap to use the new error reporting scheme (see help fordetails). The values of u and v are stored in a and b.

    >> [a, b] = loaddap('-e', 'http://www.opendap.org/opendap/nph-dods/data/fnoc1.nc?u,v');   
    Reading: http://www.opendap.org/opendap/nph-dods/data/fnoc1.nc
      Constraint: u,v
    Server version: nc/3.1.0
    >> 


OTHER CONTENTS OF THIS DIRECTORY

In addition to the two user programs, this directory includes:

writedap - Dereferences a DAP URL and returns a mixed ASCII and binarystream of data. loaddap calls writedap to perform the actual datafetch and this program must be in the same directory as loaddap. Whilethis program was originally intended to be used with loaddap, it canalso be used for other purposes as well. For example a user could savebinary data to a file with writedap. Note that with the `-f' optionwritedap translates all simple types to either Float64 or String.

writedap can also be used to read from standard input, write ASCIIoutput and access the DDS for a given dataset. See writedap's onlinehelp for information about these options.

BACKGROUND TECHNICAL INFORMATION

The following provides background technical information on the designof loaddap.

There are two programs that comprise the OPeNDAP-Matlab command lineinterface; loaddap and whodap. Both of these communicate with a thirdprogram called writedap. writedap is a DAP2 client; programs passwritedap one or more URLs along with some options, and writedapreturns the results from a DAP-enabled server.

A grammar for writedap

Note: writedap supports all of the DAP2 datatypes even though thisdocumentation doesn't show it.

Here is a pseudo grammar describing the output of writedap. I used *as the Kleene closure (zero or more) and + as one or more. The idea issimple; write the type and name of the variable each on one line anddata for it on the following line. For constructor types (Grid,Sequence, Structure) it becomes more complex, but not much. For arrayswrite Array, newline, the type, the name, the number of dimensions,newline, a list of dimension sizes, newline and the data. Finish bywriting a newline.

While this means sending out some extra characters it makes the outputof writedap very simple to parse.  Strings are individually terminatedby a newline. A second newline follows the last string.

  <data request> :: <variable>*

  <variable> :: <simple variable>
             :: <vector variable>
             :: <constructor variable>

  <simple variable> :: Byte '\n' <variable name> '\n' <data> '\n'
                    :: Int32 '\n' <variable name> '\n' <data> '\n'
                    :: Float64 '\n' <variable name> '\n'  <data> '\n'
                    :: String '\n' <variable name> '\n' <string data> '\n'
                    :: Url '\n' <variable name> '\n' <string data> '\n'

  <vector variable> :: Array '\n' <array variable> '\n' <data> [see note 1]
                    :: List '\n' <list variable> '\n' <data>

  <constructor variable> :: Structure '\n' <struct variables> 
                         :: Sequence '\n' <sequence variables>
                         :: Grid '\n' <grid variables>

  <string data> :: <string 0> '\n' <string 1> '\n' ... <string n> '\n'

  <array variable> :: <variable type> ' ' <variable name> ' ' 
                      <number of dims> '\n' <dim size>+ 

  <list variable> :: <variable type> <variable name> <list size>

  <struct variables> :: <variable name> <num of elements> '\n' 
                        (<variable>)+

  <sequence variables> :: <variable name> <num of elements> '\n' 
                          (<variable>)+

  <grid variables> :: <variable name> '\n' 'array '\n' <array variable>
                      'map <num of arrays>\n' (<array variable>)+ 

  <error> :: Error '\n' <message> '\n'


Note: The vector type's data is sent using the print_val() method ofthe vector's contained type. That is, for a vector of int32s, thecontained type is int32 and the print_val() method of the int32 classis used to send the vector's values. This method writes binary valuesand terminates them with a `\n'.  Thus, the vector variable'sprint_val() methods do not need to send the `\n'.  Since we know thatall vector and ctor types must ultimately `lead to' simple types, itis sufficient to have the simple types handle writing the `\n'separators. Having the ctors (and vectors) do this too just means moreinformation for the recipient to parse.




---------------------------------------



$Id: README 20889 2009-05-12 22:07:17Z jimg $

Updated for version 3.9.3

Now libdap supports DAP 3.2. You can read about the evolving DAP 3.x protocolat http://docs.opendap.org/index.php/DAP3/4. If your client sends theXDAP-Accept header witha value of 3.2 the DDX is different (it includesprotocol information and also an xmlbase element).

Behavior change for the DAS: In the past the format handlers added doublequotes to the values of string attributes when they added those values to theAttrTable object. This meant that the value of the attribute in the C++object was actually not correct since it contained quotes not found in theoriginal attribute value. I modified libdap so that if an attribute value inthe C++ AttrTable object does not have quotes then those quotes are addedwhen the value is output in a DAS response (but not a DDX since there's noneed to quote the value in that response). This ensures that the text in theDAS wire representation will parse whether a handler has added quotes or not(paving the way for fixed handlers). At the same time I fixed all of ourhandlers so that they no longer add the erroneous quotes. This fixes aproblem with the DDX where the quotes were showing up as part of theattribute value. The change to libdap is such that a broken handler will notbe any more broken but a fixed handler will work for both DAS and DDXgeneration.

If you have a handler and it's not adding quotes to the String attribute values - good, don't change that! If your handler does add quotes, pleasemodify it so the DDX will be correct.  

Our handler's old, broken, behavior can be resurrected by removing the ATTR_STRING_QUOTE FIX define in the appropriate files.

Updated for version 3.8.2 (23 June 2008)

HTTP Cache and win32 installer fixes (the latter are actually in the 3.8.1installer for winXP). API change: The functions used to merge ancillary datahave been moved to their own class (Ancillary).

Updated for version 3.8.1 (10 June 2008)

The syntax for PROXY_SERVER in the .dodsrc file was relaxed. See the .dodsrcfile for more information.

Updated for Version 3.8.0 (29 February 2008)

The libdap classes and code are now inside of the libdap namespace. In orderto access any of the classes, for example, you will need to do one of thefollowing. After including the libdap headers you can:

1. add a using statement for the entire libdap namespace:

using namespace libdap ;

2. add a using statement for the classes that you will be using:

using libdap::DAS ;

3. inside your code scope the use of libdap classes.

libdap::DAS *das = code_to_get_das() ;

Added method to HTTPCache to return not only the FILE pointer of a cachedresponse but also the name of the file in the cache, to allow for this filename to be passed to data handlers in the BES to be read.

See NEWS for more information about changes for this version and ChangeLogfor the gory details.

Updated for Version 3.7.10 (28 November 2007)

A bug fix release. See NEWS.

Updated for Version 3.7.9 (13 Novenber 2007)

This release is a bug fix and refactoring release. Old classes which were nolonger used have been removed, the FILE* output methods are slated to bereplaced with ones which will use iostream and will support a chuckedtransfer 'Marshaller,' and the transfer_data() methods have been made aformal part of the library, implemented for all classes, fixed and renamed tointern_data(). Many bugs in the library were also fixed.

Updated for version 3.7.8 (26 June 2007)

The major fixes in this version are memory errors found and fixed in theRegex class and HTTP header processing software. This version also supportspkg-config on hosts that have that installed.

See NEWS for more information about changes for this version and ChangeLogfor the gory details.

Notes for version 3.7.7 (2 May 2007)

The major fix here is to the source build. We've fixed the issue where sourcebuilds failed to make the dapserver and dapclient libraries.

Notes for version 3.7.6 (12 March 2007)

Two bug fixes, both minor. Problems in the linear_scale() constraintexpression function and a bad/missing #include in GNURegex.h were fixed.

There was an error in the INSTALL file sent out in the previous release. Itsaid this library implemented DAP version 3.2, but in fact it implementsversion 3.1. The version 3.2 release will be along soon (RSN).

Notes for version 3.7.5 (7 Feb 2007)

This version includes many fixes from the first Server4 beta releaseplus fixes for the server-side functions. It also includes a smootherWin32 build.

Notes for version 3.7.4 (2 Jan 2007)

Release for the Server4 beta release.

Notes for version 3.7.3 (24 Nov 2006)

This version of libdap contains a beta release of the server-side functionsgeogrid(), geoarray(), linear_scale() and version(). These can be used toselect parts of Grids and Arrays using latitude and longitude values insteadof array position indexes. The linear_scale() function can be used to scalevariables (including those return by other function) using 'y = mx + b'. Theversion() function can be used to find out which versions of the functions areinstalled.

EXAMPLES

To get version information use the 'version()' function. Currently, version()can only be called when asking for data, and you must give the name of a datasource, although in the default version of version() the data source is notused. The version function takes one optional argument which may be the strings'help' or 'xml'. Use 'help' to get help on using the function; use 'xml' to getversion information encoded using XML instead of plain text:

[jimg@zoe libdap]$ url=http://test.opendap.org/dap/data/nc/coads_climatology.nc
[jimg@zoe libdap]$ ./getdap -D "$url?version()"
The data:
String version = "Function set: version 1.0, grid 1.0, geogrid 1.0b2, 
		          geoarray 0.9b1, linear_scale 1.0b1";

[jimg@zoe libdap]$ ./getdap -D "$url?version(help)"
The data:
String version = "Usage: version() returns plain text information about ...

[jimg@zoe libdap]$ ./getdap -D "$url?version(xml)"
The data:
String version = "<?xml version=\"1.0\"?>
    <functions>
        <function name=\"version\" version=\"1.0\"/>
        <function name=\"grid\" version=\"1.0\"/>
        <function name=\"geogrid\" version=\"1.0\"/>
        <function name=\"geoarray\" version=\"1.0\"/>
        <function name=\"linear_scale\" version=\"1.0\"/>
    </functions>";

The geogrid function can only be used with variables that are Grids:

[jimg@zoe libdap]$ getdap -d "$url"
Dataset {
    Float64 COADSX[COADSX = 180];
    Float64 COADSY[COADSY = 90];
    Float64 TIME[TIME = 12];
    Grid {
      Array:
        Float32 SST[TIME = 12][COADSY = 90][COADSX = 180];
      Maps:
        Float64 TIME[TIME = 12];
        Float64 COADSY[COADSY = 90];
        Float64 COADSX[COADSX = 180];
    } SST;
    Grid {
    .
    .
    .


Pass the name of the Grid variable and the upper-left and lower-right corners of the lat/lon rectangle to geogrid. Optionally, pass one or more relationalexpressions to select parts of dimensions that are not lat/lon. 

Note: in libdap 3.7.3 calling geogrid with a constraint on each dimensionmay return incorrect values that indicate missing data even though data shouldhave been returned.

[jimg@zoe libdap]$ getdap -D "$url?geogrid(SST,30,-60,20,-60,\"TIME=366\")"
The data:
Grid {
  Array:
    Float32 SST[TIME = 1][COADSY = 7][COADSX = 2];
  Maps:
    Float64 TIME[TIME = 1];
    Float64 COADSY[COADSY = 7];
    Float64 COADSX[COADSX = 2];
} SST = {  Array: {{{24.4364, 25.0923},{23.7465, 24.4146},{19.843, 23.6033},
{16.8464, 17.7756},{16.65, 16.818},{-1e+34, 15.3656},{18.7214, 13.1286}}}  
Maps: {366}, {19, 21, 23, 25, 27, 29, 31}, {-61, -59} };


The geoarray() function works like geogrid() except that it's used to selectfrom an Array variable and not a Grid. In addition to the four lat/lon valuesfor selection rectangle, the caller must supply the data's corner points. A subsequent release of libdap will include a version that reads the data extentfrom the data source when possible so caller's won't normally have to know thedata's extent ahead of time.

The linear_scale() function take either one or three arguments. The first(only) argument is the name of a variable or the return from anotherfunction. This variable will be scaled using the 'y = mx + b' equation where'x' is the value(s) of the input variable and 'm' and 'b' are read from thedata source using the values of attributes name 'scale_factor' and'add_offset.' If these are not present, or to over ride their values, m and bcan be supplied using the second and third arguments.

Note that there are still some problems with linear_scale() in this release.

See NEWS and ChangeLog for information about other changes

Notes for version 3.7.2

This version of libdap is required for the 9/15/06 alpha release of Server4.The library now contains software which enables Server4 to build the ASCIIdata response for all types of variables, including Sequence and nestedSequence variables. These features are additions to the API, so older codewill work just fine with the new library. See NEWS for more specific infoabout bug fixes.

Notes for version 3.7.1

This is a bug fix release (mostly) made for users of the netcdf clientlibrary who need a fix for a problem dealing with attributes from the HDF4server. 

NOTES for version 3.7.0

This version includes new features and an implementation change.

This version of libdap now returns the DAP protocol version number, 3.1, inan HTTP response header. Use this to determine which protocol version thelibrary implements. The inclusion of a protocol version number is the soleofficial new feature of DAP 3.1. Use Connect::get_protocol() to get theversion number. Clients can use this to determine the features supported by aserver. The Connect::get_version() method can still be used to get ourserver's implementation version. The distinction is that as more groupsprovide their own implementations of the DAP, the protocol version willprovide a way for clients to determine capabilities independently ofimplementation.

The libdap library now contains an implementation of the DDX object/response,although this is an alpha implementation and it's actually been part of thelibrary for some time now. The implementation contained in this version ofthe library is close enough to the version we intend for DAP4 that developerscan start to use it. Most of the server handlers will return DDXs when asked.

The DDX combines the information previously held by the DDS and DAS objects,making it much easier to associate attributes to variables. As the namesuggests, the DDX uses XML rather than curly-braces. You can drop the DDXinto your favorite XML parser and get a DOM tree; no need to use our parsers.However, libdap contains a nice SAX parser that will build the libdap objectsdirectly from the XML DDX object/response. Also included in libdap aremethods to build a DDX using a DDS and DAS, so there's an easy migration pathfor both servers and clients.

Finally, the library contains two structural changes. First, the librarynamed 'libdap' now holds the DAP implementation while two new libraries,'libdapclient' and 'libdapserver', now hold the client and server helperclasses which are not strictly part of the DAP. Secondly, the DDS/DDX objectnow takes the constraint evaluator as a parameter. The classConstraintEvaluator holds our default evaluator, but it's now possible to useyour own evaluator .

NOTES for version 3.6.1

Version 3.6.1 is bug fix release.

NOTES for version 3.6.0

This version of the library may not work older source code. Many of the deprecated methods have been removed. 

Added are headers which send information about the version of the DAP protocolthat the library implements (in contrast to the implementation of the libraryitself). A new header named XOPeNDAP-Server is used to send information aboutthe implementation of servers.

The libtool interface version has been incremented from 3 to 4 (these versionsdo no track the software's release version since several releases might present compatible binary interfaces). 

NOTES for version 3.5.3

This version of libdap++ cannot be used to build the 3.4.x and previousclients and/or servers. However, client and servers built using this code_will_ work with the older clients and servers.

WHAT'S IN THIS DIRECTORY?

This directory contains the OPeNDAP C++ implementation of the DataAccess Protocol version 2 (DAP2) with some extensions that will bepart of DAP3.  Documentation for this software can be found on theOPeNDAP home page at http://www.opendap.org/. The NASA/ESE RFC whichdescribes DAP2, implemented by the library, can be found athttp://spg.gsfc.nasa.gov/rfc/004/.

The DAP2 is used to provide a uniform way of accessing a variety ofdifferent types of data across the Internet. It was originally part ofthe DODS and then NVODS projects. The focus of those projects wasaccess to Earth-Science data, so much of the software developed usingthe DAP2 to date has centered on that discipline. However, the DAP2data model is very general (and similar to a modern structuredprogramming language) so it can be applied to a wide variety offields.

The DAP2 is implemented as a set of C++ classes that can be used tobuild data servers and clients. The classes may be specialized tomimic the behavior of other data access APIs, such as netCDF. In thisway, programs originally meant to work with local data in thoseformats can be re-linked and equipped to work with data storedremotely in many different formats.  The classes can also byspecialized to build standalone client programs.

The DAP2 is contained in a single library: libdap++.a. Also includedin the library are classes and utility functions which simplifybuilding clients and servers.

WHAT ELSE IS THERE?

The file README.dodsrc describes the client-side behavior which can becontrolled using the .dodsrc file. This includes client-side caching,proxy servers, et c., and is described in a separate file so it's easyto include in your clients.

The file README.AIS describes the prototype Ancillary InformationService (AIS) included in this version of the library. The AIS is(currently) a client-side capability which provides a way to augmentDAP attributes. This is a very useful feature because it can be usedto add missing metadata to a data source. The AIS is accessed by usingthe AISConnect class in place of Connect in your client.

This directory also contains test programs for the DAP2, a samplespecialization of the classes, getdap (a useful command-line webclient created with DAP2) and dap-config (a utility script to simplifylinking with libdap.a). Also included as of version 3.5.2 islibdap.m4, an autoconf macro which developers can use along withautoconf to test for libdap. This macro will be installed in${prefix}/share/aclocal and can be by any package which uses autoconffor its builds. See the file for more information.

We also have Java and C versions of the DAP2 library whichinter-operate with software which uses this library. In other words,client programs built with the Java DAP2 implementation cancommunicate with servers built with this (C++) implementation of theDAP2. The C DAP2 library, called the Ocapi, only implements theclient-side part of the protocol. Clients written using the Ocapi areinteroperable with both the Java and C++ DAP2 libraries. Note that theOcapi is in early beta and available only from CVS at this time (5 May2005).

THREAD SAFETY

We don't need to do this since the STL is also not thread safe. Usersof libdap have to be sure that multiple threads never makesimultaneous and/or overlapping calls to a single copy of libdap. Ifseveral threads are part of a program and each will make calls tolibdap, either those threads must synchronize their calls or arrangeto each use their own copy of libdap.  Some aspects of the library''are'' thread-safe: the singleton classes are all protected as is theHTTP cache (which uses the local file system).

INSTALLATION INSTRUCTIONS

See the file INSTALL in this directory for information on building thelibrary and the geturl client.

COPYRIGHT INFORMATION

The OPeNDAP DAP library is copyrighted using the GNU Lesser GPL. Seethe file COPYING or contact the Free Software Foundation, Inc., at 59Temple Place, Suite 330, Boston, MA 02111-1307 USA. Older versions ofthe DAP were copyrighted by the University of Rhode Island andMassachusetts Institute of Technology; see the file COPYRIGHT_URI. Thefile deflate.c is also covered by COPYRIGHT_W3C.