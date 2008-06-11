
% $Id$
%
% Test 1d arrays of int16s. This test changes the way the dims matrix must be
% changed by intern() (see extend.c).

%% BROKEN jhrg 6/11/08
disp 'Test attributes from FreeForm (contains arrays of Strings): ';

load testsuite/matlab/attributes2.mat

base_url='http://test.opendap.org/opendap/ff/catalog/htn_v1.dat'

str_attr=loaddap('-A', base_url);

if isequal(str_attr_d, str_attr)
   disp 'PASS';
else
   disp 'FAIL';
end

