
% $Id: twoD_byte_array.m,v 1.1 2003/10/22 19:44:32 dan Exp $
%
% Compare data read from a network source with values previously verified.
% This test checks that byte arrays are properly processed.

disp 'Test 2d byte array: ';

load testsuite/matlab/twoD_byte_array_data.mat;

base_url = 'http://dods.gso.uri.edu/cgi-bin/nph-';

dsp_1 = [base_url 'dsp/data/east.coast?dsp_band_1[100:105][100:101]'];
loaddods(dsp_1);

if all(dsp_band_1 == dsp_band_1_data) 
   disp 'PASS';
else
   disp 'FAIL';
end

