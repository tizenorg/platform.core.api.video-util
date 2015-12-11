Name:       capi-media-video-util
Summary:    A Video Utility library in Tizen Native API
Version:    0.1.7
Release:    6
Group:      System/Libraries
License:    Apache License, Version 2.0
Source0:    %{name}-%{version}.tar.gz
Source1001: capi-media-video-util.manifest
BuildRequires:  pkgconfig(dlog)
BuildRequires:  pkgconfig(mm-common)
BuildRequires:  pkgconfig(mm-transcode)
BuildRequires:  pkgconfig(capi-base-common)
BuildRequires:  pkgconfig(capi-system-info)
BuildRequires:  pkgconfig(appcore-efl)

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
cp %{SOURCE1001} .

%build
MAJORVER=`echo %{version} | awk 'BEGIN {FS="."}{print $1}'`
%cmake . -DFULLVER=%{version} -DMAJORVER=${MAJORVER}
make %{?jobs:-j%jobs}

%install
%make_install
mkdir -p %{buildroot}/%{_datadir}/license
mkdir -p %{buildroot}/usr/bin
cp -rf %{_builddir}/%{name}-%{version}/LICENSE.APLv2.0 %{buildroot}/%{_datadir}/license/%{name}
cp test/video_util_test %{buildroot}/usr/bin/

%post

%postun

%files
%manifest %{name}.manifest
%{_libdir}/lib*.so.*
%{_datadir}/license/%{name}
/usr/bin/*

%files devel
%manifest %{name}.manifest
%{_libdir}/lib*.so
%{_libdir}/pkgconfig/*.pc
%{_includedir}/media/*.h

