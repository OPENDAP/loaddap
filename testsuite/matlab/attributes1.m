
% $Id$
%
% Test 1d arrays of int16s. This test changes the way the dims matrix must be
% changed by intern() (see extend.c).

disp 'Test attributes from fnoc2: ';

load testsuite/matlab/attributes2.mat;

base_url = 'http://test.opendap.org/dap';

fnoc2_attr = [base_url '/data/nc/fnoc2.nc'];
attr=loaddap('-A', fnoc2_attr);

if isequal(attr_d, attr)
   disp 'PASS';
else
   disp 'FAIL';
end

