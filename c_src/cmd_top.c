#include <dirent.h>
#include <getopt.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>

#include "cmds.h"
#include "libbcachefs.h"
#include "libbcachefs/sb-counters.h"

static const u8 counters_to_stable_map[] = {
#define x(n, id, ...)	[BCH_COUNTER_##n] = BCH_COUNTER_STABLE_##n,
	BCH_PERSISTENT_COUNTERS()
#undef x
};

static struct bch_ioctl_query_counters *read_counters(struct bchfs_handle fs)
{
	struct bch_ioctl_query_counters *ret =
		kzalloc(sizeof(*ret) + sizeof(ret->d[0]) * BCH_COUNTER_NR, GFP_KERNEL);

	ret->nr = BCH_COUNTER_NR;

	xioctl(fs.ioctl_fd, BCH_IOCTL_QUERY_COUNTERS, ret);
	return ret;
}

static void fs_top(const char *path, bool human_readable)
{
	struct bchfs_handle fs = bcache_fs_open(path);

	struct bch_ioctl_query_counters *curr, *prev = NULL;

	curr = read_counters(fs);

	while (true) {
		sleep(1);
		kfree(prev);
		prev = curr;
		curr = read_counters(fs);

		printf("\033[2J");
		printf("\033[H");

		for (unsigned i = 0; i < BCH_COUNTER_NR; i++) {
			unsigned stable = counters_to_stable_map[i];
			u64 v = stable < curr->nr
				?  curr->d[stable] - prev->d[stable]
				: 0;
			printf("%-48s %llu\n",
			       bch2_counter_names[i],
			       v);
		}
	}

	bcache_fs_close(fs);
}

static void fs_top_usage(void)
{
	puts("bcachefs fs top - display runtime perfomance info\n"
	     "Usage: bcachefs fs top [OPTION]... <mountpoint>\n"
	     "\n"
	     "Options:\n"
	     "  -h, --human-readable              Human readable units\n"
	     "  -H, --help                        Display this help and exit\n"
	     "Report bugs to <linux-bcachefs@vger.kernel.org>");
}

int cmd_fs_top(int argc, char *argv[])
{
	static const struct option longopts[] = {
		{ "help",		no_argument,		NULL, 'H' },
		{ "human-readable",     no_argument,            NULL, 'h' },
		{ NULL }
	};
	bool human_readable = false;
	int opt;

	while ((opt = getopt_long(argc, argv, "Hh",
				  longopts, NULL)) != -1)
		switch (opt) {
		case 'h':
			human_readable = true;
			break;
		case 'H':
			fs_top_usage();
			exit(EXIT_SUCCESS);
		default:
			fs_top_usage();
			exit(EXIT_FAILURE);
		}
	args_shift(optind);

	fs_top(arg_pop() ?: ".", human_readable) ;
	return 0;
}
