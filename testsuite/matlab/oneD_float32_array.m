
% $Id$
%
% Test 1d arrays of int16s. This test changes the way the dims matrix must be
% changed by intern() (see extend.c).

disp 'Test One D float32 array: ';

load testsuite/matlab/oneD_float32_array_data.mat;

base_url = 'http://test.opendap.org/opendap/nph-dods';

fnoc1 = [base_url '/data/nc/fnoc1.nc?lat[0:16]'];
loaddap(fnoc1);

if all(lat == lat_data)
   disp 'PASS';
else
   disp 'FAIL';
end

