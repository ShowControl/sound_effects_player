#!/bin/bash
# File: build_RPMs.sh, author: John Sauter, date: May 25, 2023.
# Build the RPMs for sound_effects_player.

# Requires fedora-packager, rpmdevtools, copr-cli.
# Don't forget to tell copr-cli about your copr API token.
# See https://developer.fedoraproject.org/deployment/copr/copr-cli.html.

rm -rf ~/rpmbuild/
mkdir -p ~/rpmbuild
mkdir -p ~/rpmbuild/SOURCES
mkdir -p ~/rpmbuild/SRPMS
mkdir -p ~/rpmbuild/RPMS/x86_64

pushd ~/rpmbuild
# Set the umask so files created will not have strange permissions.
umask 022
# Clean out any old versions of sound_effects_player.
rm -f SOURCES/*
rm -f SRPMS/*
rm -f RPMS/x86_64/*
# Copy in the new tarball and spec file.
popd
cp -v sound_effects_player-*.tar.gz ~/rpmbuild/SOURCES/
cp -v sound_effects_player.spec ~/rpmbuild/SOURCES/
pushd ~/rpmbuild/SOURCES
chmod 0644 sound_effects_player-*.tar.gz
# Build and test the source RPM.
rpmbuild -ba sound_effects_player.spec
# Copy back the source RPM so it can be copied to github.
popd
cp -v ~/rpmbuild/SRPMS/sound_effects_player-*.src.rpm .
# Perform validity checking on the RPMs.
pushd ~/rpmbuild/SRPMS
rpmlint sound_effects_player-*.src.rpm
pushd ../RPMS/x86_64/
rpmlint sound_effects_player-*.rpm
# Make sure sound_effects_player will build from the source RPM.
popd
# Disable building with Mock until I can figure out how to
# add a repository.
#mock -r fedora-33-x86_64 sound_effects_player-*.src.rpm
# now that all local tests have passed, see if it builds on copr
#copr-cli build sound_effects_player sound_effects_player-*.src.rpm

# End of file build_RPMs.sh
