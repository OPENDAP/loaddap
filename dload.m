dbmex on
%fn='http://test.opendap.org/opendap/data/nc/fnoc1.nc';
%x = loaddods('-A',fn);
base_url = 'http://test.opendap.org/opendap';

sdata = [base_url '/data/hdf/S2000415.HDF'];
sdata_attr = loaddods('-A',sdata);
