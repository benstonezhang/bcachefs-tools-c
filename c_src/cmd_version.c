#include <stdio.h>

#include "cmds.h"

int cmd_version(int argc, char *argv[])
{
	printf("%s\n", VERSION_STRING);
	return 0;
}
