
% $Id$
%
% Test use of memory by loaddods. Is there a memory leak? Test by loading 
% a variable many, many times. To use this test you must look at the size of
% the matlab's working memory with ps before and after the test is run.
% 4/19/2000 jhrg.

disp 'Test memory usage: ';

% Run the loop how many times?
test_limit = 10000;

% Use a local URL for this test. Caching will pay off here. 4/19/2000 jhrg
base_url = 'http://test.opendap.org/opendap';

fnoc1 = [base_url '/data/nc/fnoc1.nc?u'];

ii = 0;
while (ii < test_limit)
    loaddap(fnoc1);
    ii = ii + 1;
end


