

 $Id$

Updated for loaddap 3.7.0

I corrected the URLs usind in the examples and added a note regarding whodap because that command only works on the PC.

Updated for 3.6.1

Fixes for the URLs in the online documentation.

Updated for version 3.6.0

** Linux RPM USERS

You will need to add the RPMs using the --nodeps switch (rpm -Uvh --nodeps *.rpm) because the Matlab libraries that are dependencies are not in theRPM database.

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


Note: For some of the binary builds you may see a message like:

  Unable to load mex file:
  /home/xcst/codes_programms/cmex/yprime.mexglx.
  /usr/local/matlab7/bin/glnx86/../../sys/os/glnx86/libgcc_s.so.1:
  version `GCC_3.3' not found (required by /usr/lib/libstdc++.so.6)
  ??? Invalid MEX-file '/home/xcst/codes_programms/cmex/yprime.mexglx': 


This happens because some linux distributions ship with newer versions ofthe GNU gcc compiler than the version Mathworks uses to build Matlab. A fix for this (documented on the Mathworks web site) is to rename <matlab root>/sys/os/glnx86/libgcc_s.so.1 to something else such as libgcc_s.so.1.orig. With that change Matlab will now use the libgcc librarybundled with your operating system. It's better to rename the library and notdelete it just in case the version bundled with you OS doesn't work withyour copy of Maltab (in which case you can just undothe rename; you can'tuse loaddap, but at least Matlab will still work).

You may also get this error if you compile loaddap yourself. In that caseyou have two choices. Either use the above fix or get and install the sameversion of GNU's gcc compiler as was used to build your copy of Matlab and use that to build libdap and loaddap.

-------------------------------------------------------------------------------

For compilation and installation information, see the file INSTALL.

CONTENTS OF THIS DIRECTORY

The two main programs in this directory are loaddap and whodap (PConly). These are Matlab command line clients from OPeNDAP whichprovide a way to read data from DAP-enabled servers directly intoMatlab. The loaddap function reads data from an OPeNDAP server andinterns it in the Matlab workspace. The whodap function, which islimited to the Windows versions of loaddap, provides a listing of thevariables in a dataset. Both commands accept DAP URLs as arguments. 

loaddap(<dataset URL>?<variables>)

Example:

    >> loaddap('http://test.opendap.org/dap/data/nc/fnoc1.nc?u,v')
    Creating matrix u (16 x 17 x 21) with 5712 elements.
    Creating matrix v (16 x 17 x 21) with 5712 elements.


Note that you need the quotes around the argument to loaddap becauseMatlab recognizes commas as special characters.

whodap <dataset URL>

Example

    >> whodap http://test.opendap.org/dap/data/nc/fnoc1.nc
    Dataset {
	Int16 u[time_a = 16][lat = 17][lon = 21];
	Int16 v[time_a = 16][lat = 17][lon = 21];
	Float32 lat[lat = 17];
	Float32 lon[lon = 21];
	Float32 time[time = 16];
    } fnoc1;


Both whodap (PC only) and loaddap have online help (type `helploaddap' at the Matlab prompt). Loaddap has many options, alldescribed in the on-line help. It can accept multiple URLs, renamevariables, be used to read dataset attribute information and be usedin assignment statements. Here's an example of loaddap called with anoption and returning values using an assignment statement. The -eoption tells loaddap to use the new error reporting scheme (see helpfor details). The values of u and v are stored in a and b.

    >> [a, b] = loaddap('-e', 'http://test.opendap.org/dap/data/nc/fnoc1.nc?u,v');   
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



