Name:       capi-media-video-util
Summary:    A Video Utility library in Tizen Native API
Version:    0.1.6
Release:    2
Group:      System/Libraries
License:    Apache License, Version 2.0
Source0:    %{name}-%{version}.tar.gz
BuildRequires:  pkgconfig(dlog)
BuildRequires:  pkgconfig(mm-common)
BuildRequires:  pkgconfig(mm-transcode)
BuildRequires:  pkgconfig(capi-base-common)

BuildRequires:  cmake
BuildRequires:  gettext-devel

%description
A Video Utility library in Tizen Native API


%package devel
Summary:    A Video Utility library in Tizen Native API (Developement)
Group:      TO_BE_FILLED
Requires:   %{name} = %{version}-%{release}
Requires:  pkgconfig(dlog)
Requires:  pkgconfig(mm-common)
Requires:  pkgconfig(mm-transcode)
Requires:  pkgconfig(capi-base-common)

%description devel
A Video Utility library in Tizen Native API (Developement)

%prep
%setup -q

%build
MAJORVER=`echo %{version} | awk 'BEGIN {FS="."}{print $1}'`
%cmake . -DFULLVER=%{version} -DMAJORVER=${MAJORVER}
make %{?jobs:-j%jobs}

%install
%make_install
mkdir -p %{buildroot}/%{_datadir}/license
cp -rf %{_builddir}/%{name}-%{version}/LICENSE.APLv2.0 %{buildroot}/%{_datadir}/license/%{name}

%post

%postun

%files
%manifest capi-media-video-util.manifest
%{_libdir}/lib*.so.*
%{_datadir}/license/%{name}

%files devel
%{_libdir}/lib*.so
%{_libdir}/pkgconfig/*.pc
%{_includedir}/media/*.h

