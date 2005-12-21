
% Test loaddods' attribute support.
%
% $Id$
%

disp 'Test Attribute support for a dataset of arrays: ';

load testsuite/matlab/attr_arrays.mat;

base_url = 'http://test.opendap.org/opendap/nph-dods';

fnoc_url= [dcz_url '/data/nc/fnoc1.nc'];
fnoc_attr = loaddap('-A', fnoc_url);

if all(wvc_row_time__0 == wvc_row_time__0_data)
   disp 'PASS';
else
   disp 'FAIL';
end

