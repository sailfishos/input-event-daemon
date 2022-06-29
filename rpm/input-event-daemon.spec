Name:       input-event-daemon

Summary:    Streams input events from network to uinput device
Version:    0.0.0
Release:    1
Group:      System/Packages
License:    LGPLv2
URL:        https://github.com/sailfishos/input-event-daemon
Source0:    %{name}-%{version}.tar.bz2
Requires:   systemd
BuildRequires:  pkgconfig(Qt5Core)
BuildRequires:  pkgconfig(Qt5Network)
BuildRequires:  pkgconfig(systemd)

%description
The daemon streams e.g. keyboard events it receives from network connection to a uinput device. This allows emulation of hardware keys in the emulator.

%prep
%setup -q -n %{name}-%{version}

%build
%qmake5 
%make_build

%install
%qmake5_install

mkdir -p %{buildroot}%{_unitdir}/multi-user.target.wants
ln -sf ../input-event-daemon.service %{buildroot}%{_unitdir}/multi-user.target.wants/

%preun
if [ "$1" -eq 0 ]; then
systemctl stop input-event-daemon.service || :
fi

%post
systemctl daemon-reload || :
systemctl reload-or-try-restart input-event-daemon.service || :

%postun
systemctl daemon-reload || :

%files
%defattr(-,root,root,-)
%{_bindir}/input-event-daemon
%{_unitdir}/input-event-daemon.service
%{_unitdir}/multi-user.target.wants/input-event-daemon.service
