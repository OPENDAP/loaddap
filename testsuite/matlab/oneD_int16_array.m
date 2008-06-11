
% $Id$
%
% Test oneD arrays of int16s.

disp 'Test One D int16 array: ';

load testsuite/matlab/oneD_int16_array_data.mat;

base_url = 'http://test.opendap.org/opendap';

fnoc1 = [base_url '/data/nc/fnoc1.nc?u[0][0][0:20]'];
loaddap(fnoc1);

if all(u == u_data)
   disp 'PASS';
else
   disp 'FAIL';
end

