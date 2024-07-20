/*
 * Authors: Kent Overstreet <kent.overstreet@gmail.com>
 *
 * GPLv2
 */
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <uuid/uuid.h>

#include "cmds.h"
#include "libbcachefs.h"
#include "libbcachefs/errcode.h"
#include "libbcachefs/opts.h"
#include "libbcachefs/super-io.h"

static void set_option_usage(void)
{
	puts("bcachefs set-option \n"
	     "Usage: bcachefs set-option [OPTION].. device\n"
	     "\n"
	     "Options:\n");
	bch2_opts_usage(OPT_MOUNT);
	puts("  -h, --help                  display this help and exit\n"
	     "Report bugs to <linux-bcachefs@vger.kernel.org>");
	exit(EXIT_SUCCESS);
}

static int name_to_dev_idx(struct bch_fs *c, const char *dev)
{
	int ret = -1;

	rcu_read_lock();
	for_each_member_device_rcu(c, ca, NULL)
		if (!strcmp(ca->name, dev)) {
			ret = ca->dev_idx;
			break;
		}
	rcu_read_unlock();

	return ret;
}

int cmd_set_option(int argc, char *argv[])
{
	struct bch_opt_strs new_opt_strs = bch2_cmdline_opts_get(&argc, argv, OPT_MOUNT|OPT_DEVICE);
	struct bch_opts new_opts = bch2_parse_opts(new_opt_strs);
	unsigned i;
	int opt, ret = 0;

	while ((opt = getopt(argc, argv, "h")) != -1)
		switch (opt) {
		case 'h':
			set_option_usage();
			break;
		}
	args_shift(optind);

	if (!argc) {
		fprintf(stderr, "Please supply device(s)\n");
		exit(EXIT_FAILURE);
	}

	bool online = false;
	for (i = 0; i < argc; i++)
		if (dev_mounted(argv[i])) {
			online = true;
			break;
		}

	if (!online) {
		struct bch_opts open_opts = bch2_opts_empty();
		opt_set(open_opts, nostart, true);

		struct bch_fs *c = bch2_fs_open(argv, argc, open_opts);
		if (IS_ERR(c)) {
			fprintf(stderr, "error opening %s: %s\n", argv[0], bch2_err_str(PTR_ERR(c)));
			exit(EXIT_FAILURE);
		}

		mutex_lock(&c->sb_lock);

		for (i = 0; i < bch2_opts_nr; i++) {
			const struct bch_option *opt = bch2_opt_table + i;

			u64 v = bch2_opt_get_by_id(&new_opts, i);

			if (!bch2_opt_defined_by_id(&new_opts, i))
				continue;

			ret = bch2_opt_check_may_set(c, i, v);
			if (ret < 0) {
				fprintf(stderr, "error setting %s: %i\n", opt->attr.name, ret);
				continue;
			}

			if (!(opt->flags & (OPT_FS|OPT_DEVICE)))
				fprintf(stderr, "Can't set option %s\n", opt->attr.name);

			if (opt->flags & OPT_FS) {
				__bch2_opt_set_sb(c->disk_sb.sb, -1, opt, v);
			}

			if (opt->flags & OPT_DEVICE) {
				for (unsigned dev = 0; dev < argc; dev++) {
					int dev_idx = name_to_dev_idx(c, argv[dev]);
					if (dev_idx < 0) {
						fprintf(stderr, "Couldn't look up device %s\n", argv[i]);
						continue;
					}

					__bch2_opt_set_sb(c->disk_sb.sb, dev_idx, opt, v);
				}
			}
		}

		bch2_write_super(c);
		mutex_unlock(&c->sb_lock);

		bch2_fs_stop(c);
		return ret;
	} else {
		unsigned dev_idx;
		struct bchfs_handle fs = bchu_fs_open_by_dev(argv[i], &dev_idx);

		for (i = 0; i < argc; i++) {
			struct bchfs_handle fs2 = bchu_fs_open_by_dev(argv[i], &dev_idx);
			if (memcmp(&fs.uuid, &fs2.uuid, sizeof(fs2.uuid)))
				die("Filesystem mounted, but not all devices are members");
			bcache_fs_close(fs2);
		}

		for (i = 0; i < bch2_opts_nr; i++) {
			if (!new_opt_strs.by_id[i])
				continue;

			const struct bch_option *opt = bch2_opt_table + i;

			if (!(opt->flags & (OPT_FS|OPT_DEVICE)))
				fprintf(stderr, "Can't set option %s\n", opt->attr.name);

			if (opt->flags & OPT_FS) {
				char *path = mprintf("options/%s", opt->attr.name);

				write_file_str(fs.sysfs_fd, path, new_opt_strs.by_id[i]);
				free(path);
			}

			if (opt->flags & OPT_DEVICE) {
				for (unsigned dev = 0; dev < argc; dev++) {
					struct bchfs_handle fs2 = bchu_fs_open_by_dev(argv[i], &dev_idx);
					bcache_fs_close(fs2);


					char *path = mprintf("dev-%u/%s", dev_idx, opt->attr.name);
					write_file_str(fs.sysfs_fd, path, new_opt_strs.by_id[i]);
					free(path);
				}
			}
		}
	}
	return 0;
}
