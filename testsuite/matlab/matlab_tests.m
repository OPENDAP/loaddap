
% Test loaddods by reading some URLs and comparing the values read in with
% ones stored in a saved work space.
%
% $Id: matlab_tests.m,v 1.1 2003/10/22 19:44:29 dan Exp $
%

% This file contains data to compare with values read over the network. For
% each of the URLs below, a matching variable with the suffix `_data' is
% contained in this file. Compare <x> to <x>_data to determine if the read
% was successful.

load testsuite/matlab/test_loaddods.mat;

base_url = 'http://dods.gso.uri.edu/cgi-bin/nph-';

dsp_1 = [base_url 'dsp/data/east.coast?dsp_band_1[100:105][100:101]'];
loaddods(dsp_1);

if all(dsp_band_1 == dsp_band_1_data) 
   disp ' ';
   disp 'PASS: load byte array';
   disp ' ';
else
   disp ' ';
   disp 'FAIL: load byte array';
   disp ' ';
end

coads = [base_url 'nc/data/coads_climatology.nc?SST[0][0:89][0:179]'];
loaddods(coads);
% verify this.

coads2 = [base_url 'nc/data/coads_climatology.nc?SST[0][0:89][0:179] -r COADSX:Longitude -r COADSY:Latitude'];
loaddods(coads2);
%verify this.

    
fnoc1 = [base_url 'nc/data/fnoc1.nc?u[0:0][0:10][0:1],lat[0:10],time[0:0]'];
loaddods(fnoc1);

if all(u == u_data) & all(lat == lat_data) & all(time == time_data)
   disp ' ';
   disp 'PASS: load multi-varaible float arrays';
   disp ' ';
else
   disp ' ';
   disp 'FAIL: load multi-varaible float arrays';
   disp ' ';
end

fnoc2 = [base_url 'nc/data/fnoc1.nc?u[0:0][0:10][0:1] -r u:u_cat ' base_url 'nc/data/fnoc1.nc?u[1:1][0:10][0:1] -r u:u_cat'];
loaddods('-k', fnoc2);

if all(u_cat == u_cat_data)
   disp ' ';
   disp 'PASS: cat float arrays';
   disp ' ';
else
   disp ' ';
   disp 'FAIL: cat float arrays';
   disp ' ';
end

hdf1 = [base_url 'hdf/data/quikscat.hdf?wvc_row_time__0'];
loaddods(hdf1);


