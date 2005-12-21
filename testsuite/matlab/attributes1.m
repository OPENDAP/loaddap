
% $Id$
%
% Test 1d arrays of int16s. This test changes the way the dims matrix must be
% changed by intern() (see extend.c).

disp 'Test attributes from fnoc1: ';

load testsuite/matlab/attributes1.mat

base_url = 'http://test.opendap.org/opendap/nph-dods';

fnoc1_attr = [base_url '/data/nc/fnoc1.nc'];
attr=loaddap('-A', fnoc1_attr);

if isequal(attr_d, attr)
   disp 'PASS';
else
   disp 'FAIL';
end

