#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "minfs.h"

// Internal functions
pte locate_pte(FILE *img_fp, uint8_t start_sec, uint8_t part_num);
superblock locate_sb(FILE *img_fp, uint8_t start_sec);
uint32_t search_inode(FILE *img_fp, inode start, char *filename,
                      uint32_t zone_size, uint32_t lfirst);

static int v = 0;    // default: not verbose
static int part = 0; // default: no partition selection
static int sub = -1; // default: no subpartition selection

/*
 * print_help - prints a usage help message and exits
 *
 * params:
 * char* cmd - command from which help message is printed from (either minls or
 * minget)
 * */
void print_help(char *cmd) {
  char *usage_str;
  if (!strncmp(cmd, "./minls", 8)) {
    usage_str =
        "usage: minls [ -v ] [ -p num [ -s num ] ] imagefile [ path ]\n";
  } else if (!strncmp(cmd, "./minget", 9)) {
    usage_str = "usage: minget [ -v ] [ -p num [ -s num ] ] imagefile srcpath "
                "[ dstpath ]\n";
  } else {
    fprintf(stderr,
            "This utility is only usable by minls and minget currently.\n");
    return;
  }

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

/*
 * ingest_opt - ingests arguments and sets internal static values
 *
 * params:
 * int argc - argument count
 * char *argv[] - array of argument strings
 *
 * return:
 * number of options read
 * */
int ingest_opt(int argc, char *argv[]) {
  int opt_count = 0;
  int opt;
  char *optstring = "hvp:s:";
  while ((opt = getopt(argc, argv, optstring)) != -1) {
    switch (opt) {
    case 'h':
      print_help(argv[0]);
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
          if (part < 0 || part > 3) {
            fprintf(stderr, "Minix only supports partitions 0 through 3.\n");
            exit(1);
          }
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
          if (sub < 0 || sub > 3) {
            fprintf(stderr,
                    "Minix only supports sub-partitions 0 through 3.\n");
            exit(1);
          }
        }
      }
      break;
    }
  }
  return optind;
}

/*
 * find_file - locates an inode within an image
 *
 * params:
 * FILE *img_fp - location of image start. file must be open for reading binary
 * "rb" uint8_t start_sec - image starting sector (0 if searching for a
 * subpartition)
 * */
inode find_file(FILE *img_fp, char *path) {
  pte partition_pte;
  superblock sb;
  inode root;
  long ret;
  char *filename;

  // Locate page table entry for partition
  partition_pte = locate_pte(img_fp, 0, part);

  // If subpartition is set, look for the subpartition.
  if (sub != -1) {
    partition_pte = locate_pte(img_fp, partition_pte.lFirst, sub);
  }

  sb = locate_sb(img_fp, partition_pte.lFirst);

  // Allocate inode table data structure
  uint8_t inode_table_size = sizeof(inode) * sb.ninodes;
  inode *inode_table = (inode *)malloc(inode_table_size);

  // Locate the start of the inode table
  uint8_t inode_table_offset =
      ((sb.i_blocks + sb.z_blocks + 2) * sb.blocksize) * SECTOR_SIZE;

  // Seek to and read the inode table
  ret = fseek(img_fp, inode_table_offset, SEEK_SET);
  if (ret != 0) {
    fprintf(stderr, "Error on file seek.");
    free(inode_table);
    fclose(img_fp);
    exit(1);
  }
  if (fread(&inode_table_offset, inode_table_size, 1, img_fp) != 1) {
    perror("fread");
    free(inode_table);
    fclose(img_fp);
    exit(1);
  }

  // Root inode is always at inode index 1
  root = inode_table[1];
  if (v) {
    fprintf(stderr, "Root inode info:\n");
    fprintf(stderr, "-------------------------------\n");
    fprintf(stderr, "mode=%d\n", root.mode);
    fprintf(stderr, "links=%d\n", root.links);
    fprintf(stderr, "uid=%d\n", root.uid);
    fprintf(stderr, "gid=%d\n", root.gid);
    fprintf(stderr, "size=%d\n", root.size);
    fprintf(stderr, "atime=%d\n", root.atime);
    fprintf(stderr, "mtime=%d\n", root.mtime);
    fprintf(stderr, "ctime=%d\n", root.ctime);
    fprintf(stderr, "direct=%d\n", root.direct);
    fprintf(stderr, "two_indirect=%d\n", root.two_indirect);
    fprintf(stderr, "unused=%d\n\n", root.unused);
  }

  // Calculate zone size (necessary for searching)
  uint32_t zone_size = sb.blocksize << sb.log_zone_size;

  // Traverse directories starting at root inode
  char *pathdup = strdup(path);
  filename = strtok(pathdup, "/");
  inode curr_inode = root;
  uint32_t inode_index;
  while (filename) {
    char *next = strtok(NULL, "/"); // next filename in the path

    inode_index = search_inode(img_fp, curr_inode, filename, zone_size,
                               partition_pte.lFirst);
    if (inode_index == -1) {
      fprintf(stderr, "Error searching for inode.\n");
      free(inode_table);
      fclose(img_fp);
      exit(1);
    }
    curr_inode = inode_table[inode_index];

    // Only error if this node isn't the last node.
    if (next && (curr_inode.mode & I_TYPE_MASK) != I_DIRECTORY) {
      fprintf(stderr, "(%s) is not a directory.\n", filename);
      free(inode_table);
      fclose(img_fp);
      exit(1);
    }

    filename = next;
  }
  free(pathdup);

  return curr_inode;
}

/*
 * locate_pte - locates a partition table entry
 *
 * params:
 * FILE *img_fp - location of image start
 * uint8_t start_sec - image starting sector (0 if searching for a subpartition)
 * */
pte locate_pte(FILE *img_fp, uint8_t start_sector, uint8_t part_num) {
  pte pte_table[PARTITION_COUNT];
  uint8_t verify_bytes[2];
  long ret;

  // Set the file pointer to point to page table
  ret = fseek(img_fp, (start_sector * SECTOR_SIZE) + PT_OFFSET, SEEK_SET);
  if (ret != 0) {
    fprintf(stderr, "Error on file seek.");
    exit(1);
  }

  // Read in the page table
  if (fread(&pte_table, sizeof(pte), PARTITION_COUNT, img_fp) !=
      PARTITION_COUNT) {
    perror("fread");
    fclose(img_fp);
    exit(1);
  }

  // Verify the signature at the end of the boot block
  if (fread(&verify_bytes, sizeof(verify_bytes), 1, img_fp) != 1) {
    perror("fread");
    fclose(img_fp);
    exit(1);
  }

  // Verify valid start sector provided by checking the boot block signature
  if (verify_bytes[0] != BOOT_510 || verify_bytes[1] != BOOT_511) {
    fprintf(stderr, "Invalid partition signature. (0x%x, 0x%x)\n",
            verify_bytes[0], verify_bytes[1]);
    exit(1);
  }

  if (pte_table[part_num].type != MINIX_TYPE) {
    fprintf(stderr, stderr,
            "This doesn't look like a minix partition. (wrong type)\n");
    fclose(img_fp);
    exit(1);
  }

  if (v) {
    fprintf(stderr, "Partition table entry info:\n");
    fprintf(stderr, "-------------------------------\n");
    fprintf(stderr, "bootind=%d\n", pte_table[part_num].bootind);
    fprintf(stderr, "start_head=%d\n", pte_table[part_num].start_head);
    fprintf(stderr, "start_sec=%d\n", pte_table[part_num].start_sec);
    fprintf(stderr, "start_cyl=%d\n", pte_table[part_num].start_cyl);
    fprintf(stderr, "type=%d\n", pte_table[part_num].type);
    fprintf(stderr, "end_head=%d\n", pte_table[part_num].end_head);
    fprintf(stderr, "end_sec=%d\n", pte_table[part_num].end_sec);
    fprintf(stderr, "end_cyl=%d\n", pte_table[part_num].end_cyl);
    fprintf(stderr, "lFirst=%d\n", pte_table[part_num].lFirst);
    fprintf(stderr, "size=%d\n\n", pte_table[part_num].size);
  }

  return pte_table[part_num];
};

/*
 * locate_sb - locates a superblock
 *
 * params:
 * FILE *img_fp - location of image start
 * uint8_t start_sec - image starting sector
 * */
superblock locate_sb(FILE *img_fp, uint8_t start_sector) {
  long ret;
  superblock sb;

  ret = fseek(img_fp, (start_sector * SECTOR_SIZE) + SB_OFFSET, SEEK_SET);
  if (ret != 0) {
    fprintf(stderr, stderr, "Error on file seek.");
    exit(1);
  }

  // Read in the superblock
  if (fread(&sb, sizeof(superblock), 1, img_fp) != 1) {
    perror("fread");
    fclose(img_fp);
    exit(1);
  }

  // Verify the magic number
  if (sb.magic != MINIX_MAGIC) {
    fprintf(stderr, stderr, "This doesn't look like a Minix filesystem.\n");
    fclose(img_fp);
    exit(1);
  }

  if (v) {
    fprintf(stderr, "Superblock info:\n");
    fprintf(stderr, "-------------------------------\n");
    fprintf(stderr, "ninodes=:%d\n", sb.ninodes);
    fprintf(stderr, "pad1=:%d\n", sb.pad1);
    fprintf(stderr, "i_blocks=:%d\n", sb.i_blocks);
    fprintf(stderr, "z_blocks=:%d\n", sb.z_blocks);
    fprintf(stderr, "firstdata=:%d\n", sb.firstdata);
    fprintf(stderr, "log_zone_size=:%d\n", sb.log_zone_size);
    fprintf(stderr, "pad2=:%d\n", sb.pad2);
    fprintf(stderr, "max_file=:%d\n", sb.max_file);
    fprintf(stderr, "zones=:%d\n", sb.zones);
    fprintf(stderr, "magic=:%d\n", sb.magic);
    fprintf(stderr, "pad3=:%d\n", sb.pad3);
    fprintf(stderr, "blocksize=:%d\n", sb.blocksize);
    fprintf(stderr, "subversion=:%d\n\n", sb.subversion);
  }

  return sb;
}

/*
 * search_inode - searches for an inode with the given filename within the given
 * inode
 *
 * params:
 * inode start - inode where search is done
 * char* filename - filename to search for
 *
 * return:
 * uint32_t - inode number of file
 * */
uint32_t search_inode(FILE *img_fp, inode start, char *filename,
                      uint32_t zone_size, uint32_t lFirst) {
  long ret;
	int i;

  // Search all zones for a directory entry with a matching filename

  // First search all direct zones
  for (i = 0; i < DIRECT_ZONES; i++) {
    // Navigate to the zone, skip zones with 0
    if (start.zone[i] != 0) {

      // Navigate to zone
      ret = fseek(img_fp, (lFirst * SECTOR_SIZE) + (start.zone[i] * zone_size),
                  SEEK_SET);
      if (ret != 0) {
        return -1;
      }
    }
  }
}
