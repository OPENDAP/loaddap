
% $Id: oneD_int16_array.m,v 1.1 2003/10/22 19:44:31 dan Exp $
%
% Test oneD arrays of int16s.

disp 'Test One D int16 array: ';

load testsuite/matlab/oneD_int16_array_data.mat;

base_url = 'http://dods.gso.uri.edu/cgi-bin/nph-';

fnoc1 = [base_url 'nc/data/fnoc1.nc?u[0][0][0:20]'];
loaddods(fnoc1);

if all(u == u_data)
   disp 'PASS';
else
   disp 'FAIL';
end

