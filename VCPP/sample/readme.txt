
This is an example of how to run OPeNDAP's Matlab Structs Tool for
Microsoft Windows XP.

Run Matlab and issue the following commands at the Matlab prompt ">>".

>> cd C:\opendap\ml-structs
>> loaddap('http://test.opendap.org/opendap/nph-dods/data/nc/fnoc1.nc')

You should see the following response:

Creating matrix u (16 x 17 x 21) with 5712 elements.
Creating matrix v (16 x 17 x 21) with 5712 elements.
Creating vector lat with 17 elements.
Creating vector lon with 21 elements.
Creating vector time with 16 elements.

Robert Morris
October 17, 2006









