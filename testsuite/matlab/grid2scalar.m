
% Test loaddods by reading some URLs and comparing the values read in with
% ones stored in a saved work space.
%
% $Id$
%

% This file contains data to compare with values read over the network. For
% each of the URLs below, a matching variable with the suffix `_data' is
% contained in this file. Compare <x> to <x>_data to determine if the read
% was successful.

disp 'Test grid/array --> scalar constraint: ';

load testsuite/matlab/grid2scalar_data.mat;

base_url = 'http://test.opendap.org/opendap';

coads = [base_url '/data/nc/coads_climatology.nc?SST[0][0][0]'];
loaddap(coads);

if isequal(SST, SST_d)
   disp 'PASS';
else
   disp 'FAIL';
end

