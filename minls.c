#include "minfs.h"
#include "util.h"
#include <stdlib.h>
#include <string.h>

static char path[MAX_PATH_LEN];
static char imagefile[MAX_PATH_LEN];

void print_help();

int main(int argc, char *argv[]) {

  // Ingest options into the fs utility
  minfs_opts opts;
  opts = ingest_opt(argc, argv, &print_help);

  inode file_inode;

  /* Ingest minls specific options */

  // Verify there is at least one additional option to read (imagefile)
  if (opts.optind >= argc) {
    print_help();
  }

  // If there are 2  arguments left, then also read a path
  // If there is only 1 argument left, read just the image file name
  if (opts.optind == argc - 2) {
    strncpy(imagefile, argv[opts.optind], MAX_PATH_LEN);
    strncpy(path, argv[opts.optind + 1], MAX_PATH_LEN);
  } else if (opts.optind == argc - 1) {
    strncpy(imagefile, argv[opts.optind], MAX_PATH_LEN);
    strncpy(path, "/", 1); // if no path provided, default to root
  } else {
    print_help();
  }

	// Guarantee all strings are null terminated.
	imagefile[MAX_PATH_LEN - 1] = '\0';
	path[MAX_PATH_LEN - 1] = '\0';

	/* Prints parsed arguments */
  fprintf(stderr,
          "opts.v=%d\n"
          "opts.sub=%d\n"
          "opts.part=%d\n"
          "opts.sub=%d\n"
          "imagefile=%s\n"
          "path=%s\n",
          opts.v, opts.sub, opts.part, opts.sub, imagefile, path);

  // Open file for reading
  FILE *img_fp = fopen(imagefile, "rb");
  if (!img_fp) {
    printf("Failed to open imagefile.\n");
    exit(1);
  }

  // Get the inode for the file
  file_inode = find_file(img_fp, path, opts.part, opts.sub, opts.v);
}

/*
 * print_help - prints a usage help message and exits
 */
void print_help() {
  char *usage_str;
  usage_str = "usage: minls [ -v ] [ -p num [ -s num ] ] imagefile [ path ]\n";
  fprintf(stderr, "%s", usage_str);
  fprintf(stderr, "Options:\n");
  fprintf(stderr,
          "-p part\t--- select partition for filesystem (default: none)\n");
  fprintf(stderr,
          "-s sub\t--- select subpartition for filesystem (default: none)\n");
  fprintf(stderr, "-h help\t--- print usage information and exit\n");
  fprintf(stderr, "-v part\t--- increase verbosity level\n");
  exit(0);
}
