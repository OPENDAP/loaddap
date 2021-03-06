
% $Id$
%
% Test 1d arrays of int16s. This test changes the way the dims matrix must be
% changed by intern() (see extend.c).

disp 'Test loading multiple arrays: ';

load testsuite/matlab/multiple_arrays_data.mat;

base_url = 'http://test.opendap.org/opendap';

fnoc1 = [base_url '/data/nc/fnoc1.nc?lat,lon,time,u,v'];
loaddap(fnoc1);

if all(u == u_data) & all(v == v_data) & all(lat == lat_data) & all(lon == lon_data) & all(time == time_data)
   disp 'PASS';
else
   disp 'FAIL';
end

