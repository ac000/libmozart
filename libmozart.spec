Name:		libmozart
Version:	0.0.0
Release:	1%{?dist}
Summary:	Library for creating audio applications

Group:		System Environment/Libraries
License:	LGPLv3
URL:		http://github.com/ac000/libmozart
Source0:	libmozart-%{version}.tar
BuildRoot:	%(mktemp -ud %{_tmppath}/%{name}-%{version}-%{release}-XXXXXX)

BuildRequires:	gstreamer-devel >= 0.10

%description
libmozart is a library based on gstreamer for writing audio applications. It
handles various player functions and playlist manipulation.


%prep
%setup -q


%build
make %{?_smp_mflags}


%install
rm -rf $RPM_BUILD_ROOT
install -Dp -m644 libmozart.h $RPM_BUILD_ROOT/%{_includedir}/libmozart/libmozart.h
install -Dp -m644 player-operations.h $RPM_BUILD_ROOT/%{_includedir}/libmozart/player-operations.h
install -Dp -m644 playlist-operations.h $RPM_BUILD_ROOT/%{_includedir}/libmozart/playlist-operations.h
install -Dp -m644 utils.h $RPM_BUILD_ROOT/%{_includedir}/libmozart/utils.h
install -Dp -m0755 libmozart.so.%{version} $RPM_BUILD_ROOT/%{_libdir}/libmozart.so.%{version}
cd $RPM_BUILD_ROOT/%{_libdir}
ln -s libmozart.so.%{version} libmozart.so
cd -

%post -p /sbin/ldconfig

%clean
rm -rf $RPM_BUILD_ROOT


%files
%defattr(-,root,root,-)
%doc README COPYING
%{_libdir}/libmozart.*
%{_includedir}/libmozart/libmozart.h
%{_includedir}/libmozart/player-operations.h
%{_includedir}/libmozart/playlist-operations.h
%{_includedir}/libmozart/utils.h


%changelog
* Sat May 21 2011 Andrew Clayton <andrew@opentechlabs.co.uk> - 0.0.0-1
- Update for utils.h.

* Mon Jan 24 2011 Andrew Clayton <andrew@digital-domain.net> - 0.0.0-0
- Initial version.

