j = 0;

for i = 414:519
  corbit = int2str(i);
  url = ['http://dods.gso.uri.edu/cgi-bin/nph-hdf/data/S2000' corbit '.HDF?WVC_Lat[1:1][12:12]'];
  loaddap(url);
  j = j + 1;
  lat(j) = WVC_Lat / 100.0;
end
