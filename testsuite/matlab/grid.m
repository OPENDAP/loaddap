
% Test loaddods by reading some URLs and comparing the values read in with
% ones stored in a saved work space.
%
% $Id: grid.m,v 1.1 2003/10/22 19:44:29 dan Exp $
%

% This file contains data to compare with values read over the network. For
% each of the URLs below, a matching variable with the suffix `_data' is
% contained in this file. Compare <x> to <x>_data to determine if the read
% was successful.

disp 'Test grid load: ';

load testsuite/matlab/grid_data.mat;

base_url = 'http://dods.gso.uri.edu/cgi-bin/nph-';

coads = [base_url 'nc/data/coads_climatology.nc?SST'];
loaddods(coads);

if all(SST == SST_d) & all(TIME == TIME_d) & all(COADSX == COADSX_d) & all(COADSY == COADSY_d)
   disp 'PASS';
else
   disp 'FAIL';
end
