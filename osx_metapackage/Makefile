
# Build OS/X Packages. The strange operations with configure and make
# are there so that the values built into dap-config will match the mac
# osx install dirs and not the temp directory used to build the packages

PACKAGEMAKER=/Developer/Applications/Utilities/PackageMaker.app/Contents/MacOS/PackageMaker
PACKAGE=OPeNDAP_loaddap
PACKAGE_VERSION=3.7.0

pkg: pkg-clean pkg-build pkg-main pkg-dmg

pkg-clean:
	-rm -rf ${PACKAGE}-${PACKAGE_VERSION}
	-rm -rf ${PACKAGE}-${PACKAGE_VERSION}.mpkg

pkg-build:
	./build_readme.sh > ReadMe.txt
	cp ReadMe.txt Contents/product_info

pkg-main-old:
	${PACKAGEMAKER} -build -p ${PACKAGE}-${PACKAGE_VERSION}.mpkg \
		-mi ./Contents/packages \
		-r Contents/product_info \
		-i Contents/extras/Info.plist \
		-d Contents/extras/Description.plist

pkg-main:
	${PACKAGEMAKER} --root ./Contents/packages \
		--out ${PACKAGE}-${PACKAGE_VERSION}.mpkg \
		--id org.opendap.loaddap\
		--version 1 \
		--title "${PACKAGE} ${PACKAGE_VERSION}" \
		--resources Contents/product_info \
		--domain system \
		--verbose

pkg-dmg:
	mkdir ${PACKAGE}-${PACKAGE_VERSION}
	mv ${PACKAGE}-${PACKAGE_VERSION}.mpkg ${PACKAGE}-${PACKAGE_VERSION}
	cp ReadMe.First.rtf ReadMe.txt ${PACKAGE}-${PACKAGE_VERSION}
	dropdmg -i --sanitize-for-servers --format zlib \
		${PACKAGE}-${PACKAGE_VERSION}


