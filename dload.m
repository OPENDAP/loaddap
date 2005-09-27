dbmex on
%fn='http://dcz.dods.org/test-3.1/nph-nc/data/nc/fnoc1.nc';
%x = loaddods('-A',fn);
base_url = 'http://dcz.dods.org/test-3.1/nph-';

sdata = [base_url 'hdf/data/hdf/S2000415.HDF'];
sdata_attr = loaddods('-A',sdata);
