
% $Id$
%
% Test 1d arrays of int16s. This test changes the way the dims matrix must be
% changed by intern() (see extend.c).

disp 'Test 2d int16 array (with index shuffling): ';

load testsuite/matlab/twoD_int16_array3_data.mat;

base_url = 'http://test.opendap.org/opendap';

fnoc1 = [base_url '/data/nc/fnoc1.nc?u[0][0:16][0:20]'];
loaddap(fnoc1);

if all(u == u_d)
   disp 'PASS';
else
   disp 'FAIL';
end

