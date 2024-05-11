#include <getopt.h>

#include "cmds.h"
#include "libbcachefs.h"
#include "tools-util.h"
#include "libbcachefs/super.h"
#include "libbcachefs/btree_iter.h"

enum list_modes {
	MODE_KEYS,
	MODE_FORMATS,
	MODE_NODES,
	MODE_NODES_ON_DISK,
};

static int btree_id;
static enum bch_bkey_type bkey_type = KEY_TYPE_MAX;
static int list_level;
static struct bpos list_start;
static struct bpos list_end;
static enum list_modes list_mode = MODE_KEYS;
static bool run_fsck = false;
static int verbose = 0;

static void list_usage(void)
{
	puts("bcachefs list - list filesystem metadata in textual form\n"
	     "Usage: bcachefs list [options] devices\n"
	     "\n"
	     "Options:\n"
	     "  -b, --btree=   Btree to list from [default: extents]\n"
	     "  -t, --type=    Btree key type to list [default: all types]\n"
	     "  -l, --level=   Btree depth to descend to (0 = leaves) [default: 0]\n"
	     "  -s, --start=   Start position to list from [default: POS_MIN]\n"
	     "  -e, --end=     End position [default: SPOS_MAX]\n"
	     "  -m, --mode=    possible values: keys, formats, nodes, nodes-ondisk. [default: keys]\n"
	     "  -f, --fsck     Check the filesystem first\n"
	     "  -v, --verbose  Verbose mode\n"
	     "  -h, --help     Print help");
}

static int list_keys(struct bch_fs *fs)
{
	struct btree_trans *trans = bch2_trans_get(fs);
	struct btree_iter iter;
	struct bkey_s_c k;

	bch2_trans_iter_init_outlined(trans, &iter, btree_id, list_start,
				      BTREE_ITER_PREFETCH | BTREE_ITER_ALL_SNAPSHOTS);
	while (1) {
		k = bch2_btree_iter_peek_and_restart_outlined(&iter);
		if (!k.k || bpos_cmp(k.k->p, list_end) > 0)
			break;
		if ((bkey_type >= KEY_TYPE_MAX) || (k.k->type == bkey_type))
			printf("u64s=%u, format=%u, type=%u, version=%llu.%llu, "
			       "size=%u, snapshot=%u, offset=%llu, inode=%llu)\n",
			       k.k->u64s, k.k->format, k.k->type,
			       (__u64)(k.k->version.hi),
			       (__u64)(k.k->version.lo),
			       k.k->size, k.k->p.snapshot, k.k->p.offset,
			       k.k->p.inode);
		bch2_btree_iter_advance(&iter);
	}

	bch2_trans_iter_exit(trans, &iter);
	bch2_trans_put(trans);

	return 0;
}

static int list_btree_formats(struct bch_fs *fs)
{
	die("not implemented yet");
}

static int list_btree_nodes(struct bch_fs *fs)
{
	die("not implemented yet");
}

static int list_nodes_ondisk(struct bch_fs *fs)
{
	die("not implemented yet");
}

int cmd_list(int argc, char *argv[])
{
	static const struct option long_opts[] = {
		{"btree",   required_argument, NULL,        'b'},
		{"type",    required_argument, NULL,        't'},
		{"level",   required_argument, &list_level, 0},
		{"start",   required_argument, NULL,        's'},
		{"end",     required_argument, NULL,        'e'},
		{"mode",    required_argument, NULL,        'm'},
		{"fsck",    no_argument,       NULL,        'f'},
		{"verbose", no_argument,       &verbose,    1},
		{NULL}
	};
	int opt, ret = 0;

	list_start = POS_MIN;
	list_end = SPOS_MAX;

	while ((opt = getopt_long(argc, argv, "b:l:s:e:m:fv", long_opts, NULL)) != -1)
		switch (opt) {
			case 'b':
				btree_id = strtol(optarg, NULL, 10);
				break;
			case 't':
				bkey_type = (enum bch_bkey_type)strtol(optarg, NULL, 10);
				break;
			case 'l':
				list_level = strtol(optarg, NULL, 10);
				break;
			case 's':
				break;
			case 'e':
				break;
			case 'm':
				if (!strcmp(optarg, "keys"))
					list_mode = MODE_KEYS;
				else if (!strcmp(optarg, "formats"))
					list_mode = MODE_FORMATS;
				else if (!strcmp(optarg, "nodes"))
					list_mode = MODE_NODES;
				else if (!strcmp(optarg, "nodes-ondisk"))
					list_mode = MODE_NODES_ON_DISK;
				else {
					list_usage();
					exit(16);
				}
				break;
			case 'f':
				run_fsck = true;
				break;
			case 'v':
				verbose = 1;
				break;
			default:
				list_usage();
				exit(16);
		}

	args_shift(optind);

	if (!argc) {
		list_usage();
		exit(8);
	}

	darray_str devs = get_or_split_cmdline_devs(argc, argv);

	struct bch_opts opts = bch2_opts_empty();
	opt_set(opts, nochanges, true);
	opt_set(opts, read_only, true);
	opt_set(opts, norecovery, true);
	opt_set(opts, degraded, true);
	opt_set(opts, very_degraded, true);
	opt_set(opts, errors, BCH_ON_ERROR_continue);

	if (run_fsck) {
		opt_set(opts, fix_errors, FSCK_FIX_yes);
		opt_set(opts, norecovery, false);
	}
	if (verbose)
		opt_set(opts, verbose, true);

	struct bch_fs *fs = bch2_fs_open(devs.data, devs.nr, opts);
	if (IS_ERR(fs))
		die("failed open filesystem");

	switch (list_mode) {
		case MODE_KEYS:
			ret = list_keys(fs);
			break;
		case MODE_FORMATS:
			ret = list_btree_formats(fs);
			break;
		case MODE_NODES:
			ret = list_btree_nodes(fs);
			break;
		case MODE_NODES_ON_DISK:
			ret = list_nodes_ondisk(fs);
			break;
	}

	bch2_fs_stop(fs);

	darray_for_each(devs, i)
		free(*i);
	darray_exit(&devs);

	return ret;
}
