
% $Id$
%
% Test 1d arrays of int16s. This test changes the way the dims matrix must be
% changed by intern() (see extend.c).

disp 'Test One D float32 array: ';

load testsuite/matlab/oneD_float32_array_data.mat;

base_url = 'http://dods.gso.uri.edu/cgi-bin/nph-';

fnoc1 = [base_url 'nc/data/fnoc1.nc?lat[0:16]'];
loaddods(fnoc1);

if all(lat == lat_data)
   disp 'PASS';
else
   disp 'FAIL';
end

