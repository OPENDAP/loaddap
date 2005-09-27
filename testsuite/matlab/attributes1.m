
% $Id$
%
% Test 1d arrays of int16s. This test changes the way the dims matrix must be
% changed by intern() (see extend.c).

disp 'Test attributes from fnoc1: ';

load testsuite/matlab/attributes1.mat

base_url = 'http://dods.gso.uri.edu/cgi-bin/nph-';

fnoc1_attr = [base_url 'nc/data/fnoc1.nc'];
attr=loaddods('-A', fnoc1_attr);

if isequal(attr_d, attr)
   disp 'PASS';
else
   disp 'FAIL';
end

