# Tests

AUTOMAKE_OPTIONS = foreign

TESTS = writedap_test

dist_check_SCRIPTS = writedap_test atconfig atlocal

EXTRA_DIST = writedap_test.at atlocal.in $(srcdir)/package.m4 data

DISTCLEANFILES = *.log writedap_test.dir/*

############## Autotest follows #####################

AUTOM4TE = autom4te

WRITEDAP_TESTSUITE = $(srcdir)/writedap_test
WRITEDAP_TESTSUITEFLAGS =

clean-local:
	test ! -f '$(WRITEDAP_TESTSUITE)' || $(SHELL) $(WRITEDAP_TESTSUITE) --clean

distclean-local:
	-rm atconfig

AUTOTEST = $(AUTOM4TE) --language=autotest
$(WRITEDAP_TESTSUITE): $(srcdir)/writedap_test.at $(srcdir)/package.m4
	$(AUTOTEST) -I '$(srcdir)' -o $@.tmp $@.at
	mv $@.tmp $@

# The `:;' works around a Bash 3.2 bug when the output is not writable.
$(srcdir)/package.m4: $(top_srcdir)/configure.ac
	:;{ \
	echo '# Signature of the current package.' && \
	echo 'm4_define([AT_PACKAGE_NAME],      [@PACKAGE_NAME@])' && \
	echo 'm4_define([AT_PACKAGE_TARNAME],   [@PACKAGE_TARNAME@])' && \
	echo 'm4_define([AT_PACKAGE_VERSION],   [@PACKAGE_VERSION@])' && \
	echo 'm4_define([AT_PACKAGE_STRING],    [@PACKAGE_STRING@])' && \
	echo 'm4_define([AT_PACKAGE_BUGREPORT], [@PACKAGE_BUGREPORT@])'; \
	} >'$(srcdir)/package.m4'
