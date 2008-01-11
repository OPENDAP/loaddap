

 $Id$

Be sure to add /usr/local/bin to your matlabpath! For example, you could add

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



