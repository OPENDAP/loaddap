
%
% $Id$
%
% Run the matlab tests.

%% BROKEN tests comments out. jhrg 6/11/08
addpath testsuite/matlab;

disp 'Testing various datatypes';

oneD_float32_array;
oneD_int16_array;
oneD_int16_array2;
oneD_int16_array3;
twoD_int16_array;
twoD_int16_array2;
twoD_int16_array3;
threeD_int16_array;
grid;
grid2scalar;
multiple_arrays;
%% sequence;
attributes1;

% disp 'Testing various command options';

% disp 'Testing multiple loads for memory leaks';