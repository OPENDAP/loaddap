
% Test loaddods' attribute support.
%
% $Id: attr_arrays.m,v 1.1 2003/10/22 19:43:36 dan Exp $
%

disp 'Test Attribute support for a dataset of arrays: ';

load testsuite/matlab/attr_arrays.mat;

base_url = 'http://dods.gso.uri.edu/cgi-bin/nph-';

fnoc_url= [dcz_url 'nc/data/fnoc1.nc'];
fnoc_attr = loaddods('-A', fnoc_url);

if all(wvc_row_time__0 == wvc_row_time__0_data)
   disp 'PASS';
else
   disp 'FAIL';
end

