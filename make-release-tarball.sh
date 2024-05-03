#!/bin/bash

set -o errexit

version=$1

git checkout v$version
git clean -xfd

git ls-files|
    tar --create --file bcachefs-tools-$version.tar -T -	\
	--transform="s_^_bcachefs-tools-$version/_"

tar --append --file bcachefs-tools-$version.tar			\
    --transform="s_^_bcachefs-tools-$version/_"

zstd -z --ultra			bcachefs-tools-$version.tar

gpg --armor --detach-sign	bcachefs-tools-$version.tar
mv bcachefs-tools-$version.tar.asc bcachefs-tools-$version.tar.sign

gpg --armor --sign		bcachefs-tools-$version.tar

scp bcachefs-tools-$version.tar.zst	evilpiepirate.org:/var/www/htdocs/bcachefs-tools/
scp bcachefs-tools-$version.tar.asc	evilpiepirate.org:/var/www/htdocs/bcachefs-tools/
scp bcachefs-tools-$version.tar.sign	evilpiepirate.org:/var/www/htdocs/bcachefs-tools/

cp bcachefs-tools-$version.tar bcachefs-tools-vendored-$version.tar
tar --append --file bcachefs-tools-vendored-$version.tar	\
    --transform="s_^_bcachefs-tools-$version/_"			\
    .cargo vendor

zstd -z --ultra			bcachefs-tools-vendored-$version.tar

gpg --armor --detach-sign	bcachefs-tools-vendored-$version.tar
mv bcachefs-tools-vendored-$version.tar.asc bcachefs-tools-vendored-$version.tar.sign

gpg --armor --sign		bcachefs-tools-vendored-$version.tar

scp bcachefs-tools-vendored-$version.tar.zst	evilpiepirate.org:/var/www/htdocs/bcachefs-tools/
scp bcachefs-tools-vendored-$version.tar.asc	evilpiepirate.org:/var/www/htdocs/bcachefs-tools/
scp bcachefs-tools-vendored-$version.tar.sign	evilpiepirate.org:/var/www/htdocs/bcachefs-tools/
