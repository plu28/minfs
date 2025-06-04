#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "minfs.h"

static int v = 0;     // default: not verbose
static int part = -1; // default: no partition selection
static int sub = -1;  // default: no subpartition selection

void print_help(char* cmd) {
	char* usage_str;
	if (!strncmp(cmd, "./minls", 8)) {
		usage_str = "usage: minls [ -v ] [ -p num [ -s num ] ] imagefile [ path ]\n";
	} else if (!strncmp(cmd, "./minget", 9)) {
		usage_str = "usage: minget [ -v ] [ -p num [ -s num ] ] imagefile srcpath [ dstpath ]\n";
	} else {
		printf("This utility is only usable by minls and minget.\n");
		return;
	}

	printf("%s", usage_str);
	printf("Options:\n");
	printf("-p part\t--- select partition for filesystem (default: none)\n");
	printf("-s sub\t--- select subpartition for filesystem (default: none)\n");
	printf("-h help\t--- print usage information and exit\n");
	printf("-v part\t--- increase verbosity level\n");
}

void ingest_opt(int argc, char *argv[]) {
  int opt;
  char *optstring = "hvp:s:";
  while ((opt = getopt(argc, argv, optstring)) != -1) {
    switch (opt) {
    case 'h':
      print_help(argv[0]);
			exit(0);
      break;
    case 'v':
      v = 1;
      break;
    case 'p':
      if (strncmp(optarg, "?", 1)) {
        // read optarg into int
        char *end;
        long p_val = strtol(optarg, &end, 10);
        if (!strcmp(end, optarg)) {
          print_help(argv[0]);
        } else {
          part = (int)p_val;
        }
      }
      break;
    case 's':
      if (part == -1) {
        // Partition must be selected to select a subpartition
        print_help(argv[0]);
      } else if (strncmp(optarg, "?", 1)) {
        // read optarg into int
        char *end;
        long s_val = strtol(optarg, &end, 10);
        if (!strcmp(end, optarg)) {
          print_help(argv[0]);
        } else {
          sub = (int)s_val;
        }
      }
      break;
    }
  }
}

