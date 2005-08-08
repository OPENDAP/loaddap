
% Test loaddods by reading some URLs and comparing the values read in with
% ones stored in a saved work space.
%
% $Id$
%

% This file contains data to compare with values read over the network. For
% each of the URLs below, a matching variable with the suffix `_data' is
% contained in this file. Compare <x> to <x>_data to determine if the read
% was successful.

disp 'Test sequence load: ';

load testsuite/matlab/sequence_data.mat;

base_url = 'http://dods.gso.uri.edu/cgi-bin/nph-';

hdf1 = [base_url 'hdf/data/quikscat.hdf?wvc_row_time__0'];
loaddods(hdf1);

if all(wvc_row_time__0 == wvc_row_time__0_data)
   disp 'PASS';
else
   disp 'FAIL';
end

