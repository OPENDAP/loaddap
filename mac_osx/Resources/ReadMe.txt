
 $Id: ReadMe.txt 12298 2005-10-03 22:10:00Z jimg $
 
--------------------------------------------------

CONTENTS OF THIS PACKAGE

The two main programs in this directory are loaddap
and whodap. These are Matlab command line clients
from OPeNDAP which provide a way to read data from
DAP-enabled servers directly into Matlab. The loaddap
function reads data from an OPeNDAP server and
interns it in the Matlab workspace. The whodap
function provides a listing of the variables in a
dataset. Both commands accept DAP URLs as arguments.

Make sure to add /usr/local/bin and
/usr/local/share/loaddap to your matlabpath (see 'help
addpath' in Matlab). If you use an installation
directory other than /usr/local, you will need to set
the corresponding paths.

 whodap <dataset URL>

 Example

 >> whodap http://www.opendap.org/opendap/nph-dods/
           data/fnoc1.nc
    Dataset {
	Int16 u[time_a = 16][lat = 17][lon = 21];
	Int16 v[time_a = 16][lat = 17][lon = 21];
	Float32 lat[lat = 17];
	Float32 lon[lon = 21];
	Float32 time[time = 16];
    } fnoc1;

 loaddap <dataset URL>?<variables>

 Example:

 >> loaddap 'http://www.opendap.org/opendap/
             nph-dods/data/fnoc1.nc?u,v'

    Reading: http://www.opendap.org/opendap/nph-dods/
             data/fnoc1.nc
      Constraint: u,v
    Server version: nc/3.4.8
    Creating matrix u (16 x 17 x 21) with 5712 elements.
    Creating matrix v (16 x 17 x 21) with 5712 elements.

Note that you need the quotes around the argument to
loaddap because Matlab recognizes commas as special
characters.

Both whodap and loaddap have online help (type `help
loaddap' at the Matlab prompt). Loaddap has many
options, all described in the on-line help. It can
accept multiple URLs, rename variables, be used to
read dataset attribute information and be used in
assignment statements. Here's an example of loaddap
called with an option and returning values using an
assignment statement. The -e option tells loaddap to
use the new error reporting scheme (see help for
details). The values of u and v are stored in a and
b.

 >> [a, b] = loaddap('-e', 'http://www.opendap.org/
                   opendap/nph-dods/data/fnoc1.nc?u,v');   
    Reading: http://www.opendap.org/opendap/nph-dods/
             data/fnoc1.nc
      Constraint: u,v
    Server version: nc/3.1.0


OTHER CONTENTS OF THIS DIRECTORY

In addition to the two user programs, this directory includes:

writedap - Dereferences a DAP URL and returns a mixed
ASCII and binary stream of data. loaddap calls writedap
to perform the actual data fetch and this program must
be in the same directory as loaddap. While this program
was originally intended to be used with loaddap, it can
also be used for other purposes as well. For example a
user could save binary data to a file with writedap.
Note that with the `-f' option writedap translates all
simple types to either Float64 or String.

writedap can also be used to read from standard input,
write ASCII output and access the DDS for a given
dataset. See writedap's online help for information
about these options.

BACKGROUND TECHNICAL INFORMATION

The following provides background technical information
on the design of loaddap.

There are two programs that comprise the OPeNDAP-Matlab
command line interface; loaddap and whodap. Both of
these communicate with a third program called writedap.
writedap is a DAP2 client; programs pass writedap one
or more URLs along with some options, and writedap
returns the results from a DAP-enabled server.

A grammar for writedap is available with the source
code which you can get from www.OPeNDAP.org.

-*- set-fill-column: 55 -*-
