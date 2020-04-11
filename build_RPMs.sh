#!/bin/bash
# File: build_RPMs.sh, author: John Sauter, date: April 11, 2020.
# Build the RPMs for sound_effects_player

# Requires fedora-packager, rpmdevtools, copr-cli.
# Don't forget to tell copr-cli about your copr API token.
# See https://developer.fedoraproject.org/deployment/copr/copr-cli.html.

mkdir -p ~/rpmbuild
mkdir -p ~/rpmbuild/SOURCES
mkdir -p ~/rpmbuild/SRPMS
mkdir -p ~/rpmbuild/RPMS/X84_64

pushd ~/rpmbuild
# Set the umask so files created will not have strange permissions.
umask 022
# Clean out any old versions of libtime.
rm -f SOURCES/*
rm -f SRPMS/*
rm -f RPMS/x86_64/*
# Copy in the new tarball.
popd
cp -v sound_effects_player-*.tar.gz ~/rpmbuild/SOURCES/
pushd ~/rpmbuild/SOURCES
chmod 0644 sound_effects_player-*.tar.gz
# Build and test the source RPM.
rpmbuild -ta sound_effects_player-*.tar.gz
# Copy back the source RPM so it can be copied from github.
popd
cp -v ~/rpmbuild/SRPMS/sound_effects_player-*.src.rpm .
# Perform validity checking on the RPMs.
pushd ~/rpmbuild/SRPMS
rpmlint sound_effects_player-*.src.rpm
pushd ../RPMS/x86_64/
rpmlint sound_effects_player-*.rpm
# Make sure libtime will build from the source RPM.
popd
#mock -r fedora-31-x86_64 --init
#mock -r fedora-31-x86_64 --clean
#mock -r fedora-31-x86_64 --dnf-cmd install dnf-plugins-core
#mock -r fedora-31-x86_64 --dnf-cmd copr enable johnsauter/libtime
#mock -r fedora-31-x86_64 --installdeps sound_effects_player-*.src.rpm
#mock -r fedora-31-x86_64 --rebuild --no-clean sound_effects_player-*.src.rpm

# now that all local tests have passed, see if it builds on copr
copr-cli build test sound_effects_player-*.src.rpm

# End of file build_RPMs.sh
