
%
% $Id: test_loaddods.m,v 1.1 2003/10/22 19:43:35 dan Exp $
%
% Run the matlab tests.

addpath testsuite/matlab;

disp 'Testing various datatypes';

oneD_float32_array;
oneD_int16_array;
oneD_int16_array2;
oneD_int16_array3;
twoD_byte_array;
twoD_int16_array;
twoD_int16_array2;
twoD_int16_array3;
threeD_int16_array;
grid;
grid2scalar;
multiple_arrays;
sequence;
attributes1;
attributes2;

% disp 'Testing various command options';

% disp 'Testing multiple loads for memory leaks';