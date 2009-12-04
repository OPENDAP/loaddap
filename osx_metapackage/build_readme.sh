#!/bin/sh
#
# Concatenate the three ReadMe.txt files from the packages in this 
# distribution.

readme_libdap=`find Contents/packages/libdap-* -name ReadMe.txt`
readme_loaddap=`find Contents/packages/loaddap-* -name ReadMe.txt`

cat <<EOF

This package contains the OPeNDAP Matlab command loaddap and the software library it requires. What follows are the ReadMe documents from each of those packages - both lodddap and libdap are available from www.opendap.org separately.

---------------------------------------

EOF

cat $readme_loaddap

cat <<EOF

---------------------------------------

EOF

cat $readme_libdap

