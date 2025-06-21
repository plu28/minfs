#include "minfs.h"
#include "util.h"
#include <stdlib.h>
#include <string.h>

static char srcpath[MAX_PATH_LEN];
static char imagefile[MAX_PATH_LEN];
static char dstpath[MAX_PATH_LEN] =
    ""; // default: if dstpath is empty, then print to stdout

void print_help();

int main(int argc, char *argv[]) {

  // Ingest options into the fs utility
  minfs_opts opts;
  opts = ingest_opt(argc, argv, &print_help);
  inode file_inode;

  /* Ingest minget specific options */

  // Verify there are at least 2 additional options to be read (imagefile,
  // srcpath)
  if (opts.optind >= argc - 1) {
    print_help();
  }

  // If there are 3 arguments left, then also read dstpath
  // If there are 2 arguments left, then read just srcpath and imagefile
  if (opts.optind == argc - 3) {

    strncpy(imagefile, argv[opts.optind], MAX_PATH_LEN);
    strncpy(srcpath, argv[opts.optind + 1], MAX_PATH_LEN);
    strncpy(dstpath, argv[opts.optind + 2], MAX_PATH_LEN);
  } else if (opts.optind == argc - 2) {
    strncpy(imagefile, argv[opts.optind], MAX_PATH_LEN);
    strncpy(srcpath, argv[opts.optind + 1], MAX_PATH_LEN);
  } else {
    print_help();
  }
	
	// Guarantee all strings are null terminated.
	imagefile[MAX_PATH_LEN - 1] = '\0';
	srcpath[MAX_PATH_LEN - 1] = '\0';
	dstpath[MAX_PATH_LEN - 1] = '\0';

	/* Prints parsed arguments */
  // fprintf(stderr,
  //         "opts.v=%d\n"
  //         "opts.sub=%d\n"
  //         "opts.part=%d\n"
  //         "opts.sub=%d\n"
  //         "imagefile=%s\n"
  //         "srcpath=%s\n"
  //         "dstpath=%s\n",
  //         opts.v, opts.sub, opts.part, opts.sub, imagefile, srcpath,
  //         dstpath);

  // Open file for reading
  FILE *img_fp = fopen(imagefile, "rb");
  if (!img_fp) {
    printf("Failed to open imagefile.\n");
    exit(1);
  }

  // Get the inode for the file
  file_inode = find_file(img_fp, srcpath, opts.part, opts.sub, opts.v);
}

/*
 * print_help - prints a usage help message and exits
 */
void print_help() {
  char *usage_str;
  usage_str = "usage: minget [ -v ] [ -p num [ -s num ] ] imagefile srcpath "
              "[ dstpath ]\n";
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
