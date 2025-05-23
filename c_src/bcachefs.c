/*
 * Authors: Kent Overstreet <kent.overstreet@gmail.com>
 *	    Gabriel de Perthuis <g2p.code@gmail.com>
 *	    Jacob Malevich <jam@datera.io>
 *
 * GPLv2
 */

#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <errno.h>
#include <inttypes.h>
#include <limits.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <raid/raid.h>

#include "cmds.h"

void bcachefs_usage(void)
{
	puts("bcachefs - tool for managing bcachefs filesystems\n"
	     "usage: bcachefs <command> [<args>]\n"
	     "\n"
	     "Superblock commands:\n"
	     "  format                   Format a new filesystem\n"
	     "  show-super               Dump superblock information to stdout\n"
	     "  recover-super            Attempt to recover overwritten superblock from backups\n"
	     "  set-fs-option            Set a filesystem option\n"
	     "  reset-counters           Reset all counters on an unmounted device\n"
	     "\n"
	     "Mount:\n"
	     "  mount                    Mount a filesystem\n"
	     "\n"
	     "Repair:\n"
	     "  fsck                     Check an existing filesystem for errors\n"
	     "\n"
#if 0
	     "Startup/shutdown, assembly of multi device filesystems:\n"
	     "  assemble                 Assemble an existing multi device filesystem\n"
	     "  incremental              Incrementally assemble an existing multi device filesystem\n"
	     "  run                      Start a partially assembled filesystem\n"
	     "  stop	                 Stop a running filesystem\n"
	     "\n"
#endif
	     "Commands for managing a running filesystem:\n"
	     "  fs usage                 Show disk usage\n"
	     "  fs top                   Show runtime performance information\n"
	     "\n"
	     "Commands for managing devices within a running filesystem:\n"
	     "  device add               Add a new device to an existing filesystem\n"
	     "  device remove            Remove a device from an existing filesystem\n"
	     "  device online            Re-add an existing member to a filesystem\n"
	     "  device offline           Take a device offline, without removing it\n"
	     "  device evacuate          Migrate data off of a specific device\n"
	     "  device set-state         Mark a device as failed\n"
	     "  device resize            Resize filesystem on a device\n"
	     "  device resize-journal    Resize journal on a device\n"
	     "\n"
	     "Commands for managing subvolumes and snapshots:\n"
	     "  subvolume create         Create a new subvolume\n"
	     "  subvolume delete         Delete an existing subvolume\n"
	     "  subvolume snapshot       Create a snapshot\n"
	     "\n"
	     "Commands for managing filesystem data:\n"
	     "  data rereplicate         Rereplicate degraded data\n"
	     "  data scrub               Verify checksums and correct errors, if possible\n"
	     "  data job                 Kick off low level data jobs\n"
	     "\n"
	     "Encryption:\n"
	     "  unlock                   Unlock an encrypted filesystem prior to running/mounting\n"
	     "  set-passphrase           Change passphrase on an existing (unmounted) filesystem\n"
	     "  remove-passphrase        Remove passphrase on an existing (unmounted) filesystem\n"
	     "\n"
	     "Migrate:\n"
	     "  migrate                  Migrate an existing filesystem to bcachefs, in place\n"
	     "  migrate-superblock       Add default superblock, after bcachefs migrate\n"
	     "\n"
	     "Commands for operating on files in a bcachefs filesystem:\n"
	     "  set-file-option          Set various attributes on files or directories\n"
	     "\n"
	     "Debug:\n"
	     "These commands work on offline, unmounted filesystems\n"
	     "  dump                     Dump filesystem metadata to a qcow2 image\n"
	     "  list                     List filesystem metadata in textual form\n"
	     "  list_journal             List contents of journal\n"
	     "\n"
#ifdef BCACHEFS_FUSE
	     "FUSE:\n"
	     "  fusemount                Mount a filesystem via FUSE\n"
	     "\n"
#endif
	     "Miscellaneous:\n"
	     "  completions              Generate shell completions\n"
	     "  version                  Display the version of the invoked bcachefs tool\n");
}

int fs_cmds(int argc, char *argv[])
{
	char *cmd = pop_cmd(&argc, argv);

	if (argc < 1)
		return fs_usage();
	if (!strcmp(cmd, "usage"))
		return cmd_fs_usage(argc, argv);
	if (!strcmp(cmd, "top"))
		return cmd_fs_top(argc, argv);

	fs_usage();
	return -EINVAL;
}

int main(int argc, char *argv[])
{
	raid_init();

	setvbuf(stdout, NULL, _IOLBF, 0);

	char *full_cmd = argv[0];

	/* Are we being called via a symlink? */

	if (strstr(full_cmd, "mkfs"))
		return cmd_format(argc, argv);

	if (strstr(full_cmd, "fsck"))
		return cmd_fsck(argc, argv);

#ifdef BCACHEFS_FUSE
	if (strstr(full_cmd, "mount.fuse"))
		return cmd_fusemount(argc, argv);
#endif

	if (strstr(full_cmd, "mount"))
		return cmd_mount(argc, argv);

	char *cmd = pop_cmd(&argc, argv);
	if (!cmd) {
		puts("missing command\n");
		goto usage;
	}

	/* these subcommands display usage when argc < 2 */
	if (!strcmp(cmd, "device"))
		return device_cmds(argc, argv);
	if (!strcmp(cmd, "fs"))
		return fs_cmds(argc, argv);
	if (!strcmp(cmd, "data"))
		return data_cmds(argc, argv);
	if (!strcmp(cmd, "subvolume"))
		return subvolume_cmds(argc, argv);
	if (!strcmp(cmd, "format"))
		return cmd_format(argc, argv);
	if (!strcmp(cmd, "fsck"))
		return cmd_fsck(argc, argv);
	if (!strcmp(cmd, "version"))
		return cmd_version(argc, argv);
	if (!strcmp(cmd, "show-super"))
		return cmd_show_super(argc, argv);
	if (!strcmp(cmd, "set-option"))
		return cmd_set_option(argc, argv);
	if (!strcmp(cmd, "reset-counters"))
		return cmd_reset_counters(argc, argv);

#if 0
	if (!strcmp(cmd, "assemble"))
		return cmd_assemble(argc, argv);
	if (!strcmp(cmd, "incremental"))
		return cmd_incremental(argc, argv);
	if (!strcmp(cmd, "run"))
		return cmd_run(argc, argv);
	if (!strcmp(cmd, "stop"))
		return cmd_stop(argc, argv);
#endif

	if (!strcmp(cmd, "unlock"))
		return cmd_unlock(argc, argv);
	if (!strcmp(cmd, "set-passphrase"))
		return cmd_set_passphrase(argc, argv);
	if (!strcmp(cmd, "remove-passphrase"))
		return cmd_remove_passphrase(argc, argv);

	if (!strcmp(cmd, "migrate"))
		return cmd_migrate(argc, argv);
	if (!strcmp(cmd, "migrate-superblock"))
		return cmd_migrate_superblock(argc, argv);

	if (!strcmp(cmd, "dump"))
		return cmd_dump(argc, argv);
	if (!strcmp(cmd, "list"))
		return cmd_list(argc, argv);
	if (!strcmp(cmd, "list_journal"))
		return cmd_list_journal(argc, argv);

	if (!strcmp(cmd, "setattr"))
		return cmd_setattr(argc, argv);

	if (!strcmp(cmd, "completions"))
		return cmd_completions(argc, argv);

#ifdef BCACHEFS_FUSE
	if (!strcmp(cmd, "fusemount"))
		return cmd_fusemount(argc, argv);
#endif

	if (!strcmp(cmd, "mount"))
		return cmd_mount(argc, argv);

	if (!strcmp(cmd, "--help")) {
		bcachefs_usage();
		return 0;
	}

	printf("Unknown command %s\n", cmd);
usage:
	bcachefs_usage();
	exit(EXIT_FAILURE);
}
