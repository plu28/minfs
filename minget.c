#include "minfs.h"
#include <stdlib.h>
#include <string.h>

static char path[MAX_PATH_LEN];
static char imagefile[MAX_FILE_NAME_LEN];
static char dstfile[MAX_FILE_NAME_LEN];

int main(int argc, char *argv[]) {
  // Ingest options into the fs utility
  int optind;
  optind = ingest_opt(argc, argv);
  inode file_inode;

  // Ingest commmand specific options after options by using optind
  // Verify there are actually values at optind
  if (optind >= argc) {
    print_help("./minls");
  }

  // If there are 2  arguments left, then also read a path
  // If there is only 1 argument left, read just the image file name
  if (optind == argc - 2) {
    strncpy(imagefile, argv[optind], MAX_FILE_NAME_LEN);
    strncpy(path, argv[optind + 1], MAX_FILE_NAME_LEN);
  } else if (optind == argc - 1) {
    strncpy(imagefile, argv[optind], MAX_FILE_NAME_LEN);
    strncpy(path, "/", 1); // if no path provided, default to root
  }

  // Open file for reading
  FILE *img_fp = fopen(imagefile, "rb");
  if (!img_fp) {
    printf("Failed to open imagefile.\n");
    exit(1);
  }

  // Get the inode for the file
  file_inode = find_file(img_fp, path);
}
