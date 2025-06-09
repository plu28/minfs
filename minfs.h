#ifndef MINFS_H
#define MINFS_H
#include <stdint.h>
#include <stdio.h>

#define SECTOR_SIZE 512            // size of a sector
#define PT_OFFSET 0x1BE            // partition table offset within boot block
#define SB_OFFSET 1024             // byte offset for superblock
#define PT_OFFSET 0x1BE            // partition table offset within boot block
#define MINIX_TYPE 0x81            // partition type for minix
#define SIGN_P1 510                // 1st signature byte to check
#define SIGN_P2 511                // 2nd signature byte to check
#define BOOT_510 0x55              // byte 510 of boot sector with valid pte
#define BOOT_511 0xAA              // byte 511 of boot sector with valid pte
#define MINIX_MAGIC 0x4D5A         // Minix magic number
#define MINIX_MAGIC_REVERSE 0x5A4D // Minix magic number on big endian system
#define INODE_S 64                 // Size of inode
#define DIR_ENTRY_S 64             // Size of directory entry
#define DIRECT_ZONES 7             // Number of direct zones in an inode
#define PARTITION_COUNT 4          // Number of partitions per partition table

// Limits defined by Minix
#define MAX_PATH_LEN 1024
#define MAX_FILE_NAME_LEN 60

// File type masks for inodes
#define I_TYPE_MASK 0170000
#define I_REGULAR 0100000
#define I_DIRECTORY 0040000
#define I_DIRECTORY 0040000
#define I_OWNER_READ 0000400
#define I_OWNER_WRITE 0000200
#define I_OWNER_EXECUTE 0000100
#define I_GROUP_READ 0000040
#define I_GROUP_WRITE 0000020
#define I_GROUP_EXECUTE 0000010
#define I_OTHER_READ 0000004
#define I_OTHER_WRITE 0000002
#define I_OTHER_EXECUTE 0000001

typedef struct __attribute__((packed)) pte {
  uint8_t bootind;
  uint8_t start_head;
  uint8_t start_sec;
  uint8_t start_cyl;
  uint8_t type;
  uint8_t end_head;
  uint8_t end_sec;
  uint8_t end_cyl;
  uint32_t lFirst;
  uint32_t size;
} pte;

typedef struct __attribute__((packed)) inode {
  uint16_t mode;
  uint16_t links;
  uint16_t uid;
  uint16_t gid;
  uint32_t size;
  uint32_t atime;
  uint32_t mtime;
  uint32_t ctime;
  uint32_t zone[DIRECT_ZONES];
  uint32_t direct;
  uint32_t two_indirect;
  uint32_t unused;
} inode;

typedef struct __attribute__((packed)) superblock {
  uint32_t ninodes;
  uint16_t pad1;
  int16_t i_blocks;
  int16_t z_blocks;
  uint16_t firstdata;
  int16_t log_zone_size;
  int16_t pad2;
  uint32_t max_file;
  uint32_t zones;
  int16_t magic;
  int16_t pad3;
  uint16_t blocksize;
  uint8_t subversion;
} superblock;

typedef struct __attribute__((packed)) dir_entry {
  uint32_t inode;
  unsigned char name[MAX_FILE_NAME_LEN]; // NOTE: if strnlen(name) == 60, the
                                         // string has no null terminator.
} dir_entry;

int ingest_opt(int argc, char *argv[]);
void print_help(char *);
inode find_file(FILE *img_fp, char *path);

#endif
