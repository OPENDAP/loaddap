
Name: loaddap
Summary: The OPeNDAP Matlab Command Line Interface Client
Version: 3.5.0
Release: 1

Source0: http://www.opendap.org/pub/3.5/source/%{name}-%{version}.tar.gz
URL: http://www.opendap.org/

Group: Development/Libraries
BuildRoot:  %{_tmppath}/%{name}-%{version}-%{release}-root-%(%{__id_u} -n)
License: LGPL/W3C
Requires: libdap >= 3.5.0
# Mandrake
#BuildRequires: libcurl3-devel >= 7.10.6 libxml2-devel >= 2.5.7
# fedora
#BuildRequires: libdap-devel >= 3.5.2

# This package could be relocatable. In that case uncomment the following
# line
Prefix: %{_prefix}

%description
This package contains the OPeNDAP Matlab command line interface client. 
This client can be used to read data from DAP2-compilant servers directly
into Matlab. This version of the client has been tested witrh Matlab 7,
although previous versions are goin gback to version 5 are likely to work.

%prep
%setup -q

%build
%configure
make %{?_smp_mflags}

%install
rm -rf $RPM_BUILD_ROOT
make install DESTDIR=$RPM_BUILD_ROOT
rm -f $RPM_BUILD_ROOT/%{_libdir}/*.la

%clean
rm -rf $RPM_BUILD_ROOT

%files
%defattr(-,root,root,-)
%{_bindir}/loaddap.mex*
%{_bindir}/writedap
%{_datadir}/loaddap/loaddap.m
%{_datadir}/loaddap/whodap.m
%doc README NEWS COPYING COPYRIGHT_URI

%changelog
* Mon Oct 3  2005 James Gallagher <jgallagher@opendap.org> - 3.5.0-1
- Added
