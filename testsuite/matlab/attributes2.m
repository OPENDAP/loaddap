
% $Id: attributes2.m,v 1.1 2003/10/22 19:43:37 dan Exp $
%
% Test 1d arrays of int16s. This test changes the way the dims matrix must be
% changed by intern() (see extend.c).

disp 'Test attributes from FreeForm (contains arrays of Strings): ';

load testsuite/matlab/attributes2.mat

base_url='http://maewest.gso.uri.edu/cgi-bin/nph-ff/catalog/htn_v1.dat'

str_attr=loaddods('-A', base_url);

if isequal(str_attr_d, str_attr)
   disp 'PASS';
else
   disp 'FAIL';
end

