# 
# spec file for tohcom
# 

Name:       tohcom

%{!?qtc_qmake:%define qtc_qmake %qmake}
%{!?qtc_qmake5:%define qtc_qmake5 %qmake5}
%{!?qtc_make:%define qtc_make make}
%{?qtc_builddir:%define _builddir %qtc_builddir}

Summary:    TOHUART Serial Console
Version:    0.0.2
Release:    devel
Group:      Qt/Qt
License:    LICENSE
URL:        https://github.com/kimmoli/tohcom
Source0:    %{name}-%{version}.tar.bz2

BuildRequires:  pkgconfig(Qt5Core)
BuildRequires:  pkgconfig(Qt5DBus)
BuildRequires:  desktop-file-utils

%description
Serial Console for TOHUART, TheOtherHalf UART

%prep
%setup -q -n %{name}-%{version}

%build

%qtc_qmake5 SPECVERSION=%{version}

%qtc_make %{?_smp_mflags}

%install
rm -rf %{buildroot}

%qmake5_install

desktop-file-install --delete-original       \
  --dir %{buildroot}%{_datadir}/applications             \
   %{buildroot}%{_datadir}/applications/*.desktop

%files
%defattr(644,root,root,755)
%attr(6755,root,root) %{_bindir}/tohcom
%attr(755,root,root) %{_bindir}/picocom
%{_datadir}/applications
%{_datadir}/icons/hicolor/86x86/apps/
%{_datadir}/dbus-1/system-services/
%{_sysconfdir}/dbus-1/system.d/
