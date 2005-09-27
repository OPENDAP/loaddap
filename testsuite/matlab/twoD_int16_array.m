
% $Id$
%
% Test 1d arrays of int16s. This test changes the way the dims matrix must be
% changed by intern() (see extend.c).

disp 'Test 2d int16 array: ';

load testsuite/matlab/twoD_int16_array_data.mat;

base_url = 'http://dods.gso.uri.edu/cgi-bin/nph-';

fnoc1 = [base_url 'nc/data/fnoc1.nc?u[0:15][0:16][0]'];
loaddods(fnoc1);

if all(u == u_data)
   disp 'PASS';
else
   disp 'FAIL';
end

