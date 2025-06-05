#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "minfs.h"

static int v = 0;     // default: not verbose
static int part = 0; // default: no partition selection
static int sub = -1;  // default: no subpartition selection

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
    printf("This utility is only usable by minls and minget.\n");
    return;
  }

  printf("%s", usage_str);
  printf("Options:\n");
  printf("-p part\t--- select partition for filesystem (default: none)\n");
  printf("-s sub\t--- select subpartition for filesystem (default: none)\n");
  printf("-h help\t--- print usage information and exit\n");
  printf("-v part\t--- increase verbosity level\n");
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
            printf("Minix only supports partitions 0 through 3.\n");
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
            printf("Minix only supports sub-partitions 0 through 3.\n");
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

  // Locate page table entry for partition
  partition_pte = locate_pte(img_fp, 0, part);

  // If subpartition is set, look for the subpartition.
  if (sub != -1) {
    partition_pte = locate_pte(img_fp, partition_pte.lFirst, sub);
  }


	sb = locate_sb(img_fp, partition_pte.lFirst);
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
    printf("Error on file seek.");
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
    printf("Invalid partition signature. (0x%x, 0x%x)\n", verify_bytes[0],
           verify_bytes[1]);
    exit(1);
  }

	if (pte_table[part_num].type != MINIX_TYPE) {
		printf("This doesn't look like a minix partition. (wrong type)\n");
		fclose(img_fp);
		exit(1);
	}
	
	if (v) {
		printf("Partition table entry info:\n");
		printf("-------------------------------\n");
		printf("bootind=%d\n", pte_table[part_num].bootind);
		printf("start_head=%d\n", pte_table[part_num].start_head);
		printf("start_sec=%d\n", pte_table[part_num].start_sec);
		printf("start_cyl=%d\n", pte_table[part_num].start_cyl);
		printf("type=%d\n", pte_table[part_num].type);
		printf("end_head=%d\n", pte_table[part_num].end_head);
		printf("end_sec=%d\n", pte_table[part_num].end_sec);
		printf("end_cyl=%d\n", pte_table[part_num].end_cyl);
		printf("lFirst=%d\n", pte_table[part_num].lFirst);
		printf("size=%d\n\n", pte_table[part_num].size);
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
superblock locate_sb(FILE* img_fp, uint8_t start_sector) {
  long ret;
	superblock sb;

  ret = fseek(img_fp, (start_sector * SECTOR_SIZE) + SB_OFFSET, SEEK_SET);
  if (ret != 0) {
    printf("Error on file seek.");
    exit(1);
  }

	// Read in the superblock
  if (fread(&sb, sizeof(sb), 1, img_fp) !=
      1) {
    perror("fread");
    fclose(img_fp);
    exit(1);
  }

	// Verify the magic number
	if (sb.magic != MINIX_MAGIC) {
		printf("This doesn't look like a Minix filesystem.\n");
		fclose(img_fp);
		exit(1);
	}

	if (v) {
		printf("Superblock info:\n");
		printf("-------------------------------\n");
		printf("ninodes=:%d\n", sb.ninodes);
		printf("pad1=:%d\n", sb.pad1);
		printf("i_blocks=:%d\n", sb.i_blocks);
		printf("z_blocks=:%d\n", sb.z_blocks);
		printf("firstdata=:%d\n", sb.firstdata);
		printf("log_zone_size=:%d\n", sb.log_zone_size);
		printf("pad2=:%d\n", sb.pad2);
		printf("max_file=:%d\n", sb.max_file);
		printf("zones=:%d\n", sb.zones);
		printf("magic=:%d\n", sb.magic);
		printf("pad3=:%d\n", sb.pad3);
		printf("blocksize=:%d\n", sb.blocksize);
		printf("subversion=:%d\n\n", sb.subversion);
	}

	return sb;

}
