
%
% $Id$
%
% Run the matlab tests.

%% BROKEN tests comments out. jhrg 6/11/08

disp(['If this script returns an error about files not found, be ' ...
      'sure to add the .../testsuite/matlab directory to matlabpath ' ...
      'using the addpath command.'])

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
sequence;
attributes1;
