
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

base_url = 'http://test.opendap.org/opendap';
%opendap/hyrax/data/ff/1998-6-avhrr.dat?GSO_AVHRR.year,GSO_AVHRR.day_num,GSO_AVHRR.DODS_URL
ff1 = [base_url '/data/ff/1998-6-avhrr.dat?year,day_num,DODS_URL'];
loaddap(ff1);

if isequal(GSO_AVHRR, GSO_AVHRR_d)
   disp 'PASS';
else
   disp 'FAIL';
end

