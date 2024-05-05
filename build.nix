{ lib, stdenv, pkg-config, attr, libuuid, libsodium, keyutils, liburcu, zlib
, libaio, udev, zstd, lz4, nix-gitignore, fuse3
, fuseSupport ? false, }:
let
  src = nix-gitignore.gitignoreSource [ ] ./.;

  commit = lib.strings.substring 0 7 (builtins.readFile ./.bcachefs_revision);
  version = "git-${commit}";
in stdenv.mkDerivation {
  inherit src version;

  pname = "bcachefs-tools";

  nativeBuildInputs = [
    pkg-config
  ];

  buildInputs = [
    libaio
    keyutils # libkeyutils
    lz4 # liblz4

    libsodium
    liburcu
    libuuid
    zstd # libzstd
    zlib # zlib1g
    attr
    udev
  ] ++ lib.optional fuseSupport fuse3;

  ${if fuseSupport then "BCACHEFS_FUSE" else null} = "1";

  makeFlags = [ "DESTDIR=${placeholder "out"}" "PREFIX=" "VERSION=${commit}" ];

  dontStrip = true;
  checkPhase = "./bcachefs version";
  doCheck = true;

  meta = {
    mainProgram = "bcachefs";
    license = lib.licenses.gpl2Only;
  };
}
