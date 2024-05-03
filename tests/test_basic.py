#!/usr/bin/python3
#
# Basic bcachefs functionality tests.

import re
from tests import util

def test_help():
    ret = util.run_bch(valgrind=True)

    assert ret.returncode == 1
    assert "missing command" in ret.stdout
    assert len(ret.stderr) == 0

def test_format(tmpdir):
    dev = util.device_1g(tmpdir)
    ret = util.run_bch('format', dev, valgrind=True)

    assert ret.returncode == 0
    assert len(ret.stdout) > 0
    assert len(ret.stderr) == 0

def test_fsck(tmpdir):
    dev = util.format_1g(tmpdir)

    ret = util.run_bch('fsck', dev, valgrind=True)

    assert ret.returncode == 0
    assert len(ret.stdout) > 0
    assert len(ret.stderr) == 0

def test_list(tmpdir):
    dev = util.format_1g(tmpdir)

    ret = util.run_bch('list', dev, valgrind=True)

    assert ret.returncode == 0
    assert len(ret.stderr) == 0
    assert "recovering from clean shutdown" in ret.stdout

def test_list_inodes(tmpdir):
    dev = util.format_1g(tmpdir)

    ret = util.run_bch('list', '-b', 'inodes', dev, valgrind=True)

    assert ret.returncode == 0
    assert len(ret.stderr) == 0
    assert len(ret.stdout.splitlines()) == (67)

def test_list_dirent(tmpdir):
    dev = util.format_1g(tmpdir)

    ret = util.run_bch('list', '-b', 'dirents', dev, valgrind=True)

    assert ret.returncode == 0
    assert len(ret.stderr) == 0
    assert len(ret.stdout.splitlines()) == (6) # See example:

    # Example:
    # mounting version 1.6: btree_subvolume_children opts=ro,errors=continue,degraded,nochanges,norecovery,read_only
    # recovering from clean shutdown, journal seq 9
    # alloc_read... done
    # stripes_read... done
    # snapshots_read... done
    # u64s 8 type dirent 4096:453699834857023875:U32_MAX len 0 ver 0: lost+found -> 4097 type dir
    last = ret.stdout.splitlines()[0]
    assert re.match(r'^.*type dirent.*: lost\+found ->.*$', last)
