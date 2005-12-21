
% $Id$
%
% Compare data read from a network source with values previously verified.
% This test checks that byte arrays are properly processed.

disp 'Test 2d byte array: ';

load testsuite/matlab/twoD_byte_array_data.mat;

base_url = 'http://test.opendap.org/opendap/nph-dods';

dsp_1 = [base_url '/data/dsp/east.coast?dsp_band_1[100:105][100:101]'];
loaddap(dsp_1);

if all(dsp_band_1 == dsp_band_1_data) 
   disp 'PASS';
else
   disp 'FAIL';
end

