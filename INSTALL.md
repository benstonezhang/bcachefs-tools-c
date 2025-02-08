Getting started
---------------

Build dependencies:

 * libaio
 * libblkid
 * libclang
 * libkeyutils
 * liblz4
 * libsodium
 * liburcu
 * libuuid
 * libzstd
 * pkg-config
 * valgrind
 * zlib1g

Debian (Bullseye or later) and Ubuntu (20.04 or later): you can install these with

``` shell
apt install -y pkg-config libaio-dev libblkid-dev libkeyutils-dev \
    liblz4-dev libsodium-dev liburcu-dev libzstd-dev \
    uuid-dev zlib1g-dev valgrind libudev-dev udev git build-essential \
    python3 python3-docutils libclang-dev debhelper dh-python
```

Starting from Debian Trixie and Ubuntu 23.10, you will additionally need:
```shell
apt install -y systemd-dev
```

Fedora: install the "Development tools" group along with:
```shell
dnf install -y libaio-devel libsodium-devel \
    libblkid-devel libzstd-devel zlib-devel userspace-rcu-devel \
    lz4-devel libuuid-devel valgrind-devel keyutils-libs-devel \
    findutils udev systemd-devel llvm-devel
```

openSUSE: install build dependencies with:
```shell
zypper in -y libaio-devel libsodium-devel libblkid-devel liburcu-devel \
    libzstd-devel zlib-devel liblz4-devel libuuid-devel valgrind-devel \
    keyutils-devel findutils udev systemd-devel llvm-devel
```

Arch: install bcachefs-tools-git from the AUR.
Or to build from source, install build dependencies with
```shell
pacman -S base-devel libaio keyutils libsodium liburcu zstd valgrind llvm
```

Then, just `make && make install`


Experimental features
---------------------

Experimental fuse support is currently disabled by default. Fuse support is at
an early stage and may corrupt your filesystem, so it should only be used for
testing. To enable, you'll also need to add:

* libfuse3 >= 3.7

On Debian/Ubuntu (Bullseye/20.04 or later needed for libfuse >= 3.7):
```shell
apt install -y libfuse3-dev
```

On Fedora (32 or later needed for libfuse >= 3.7):
```shell
dnf install -y fuse3-devel
```

Arch:
```shell
pacman -S fuse3
```

openSUSE:
```shell
zypper in -y fuse3-devel
```

Then, make using the `BCACHEFS_FUSE` environment variable (make clean first if
previously built without fuse support):

```shell
BCACHEFS_FUSE=1 make && make install
```
