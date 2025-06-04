#ifndef MINFS_H
#define MINFS_H
#include <stdint.h>
#include <stdio.h>

#define PT_OFFSET 0x1BE // partition table offset within boot block
#define MINIX_TYPE 0x81 // partition type for minix
#define BOOT_510 0x55		// byte 510 of boot sector with valid pte
#define BOOT_511 0xAA		// byte 511 of boot sector with valid pte
#define MINIX_MAGIC 0x4D5A // Minix magic number
#define MINIX_MAGIC_REVERSE 0x5A4D // Minix magic number on a byte reversed system (big endian)
#define INODE_S 64 // Size of inode 
#define DIR_ENTRY_S 64 // Size of directory entry
#define DIRECT_ZONES 7 // Number of direct zones in an inode

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

typedef struct __attribute__ ((packed)) dir_entry {
	uint32_t inode;
	unsigned char name[60]; // NOTE: if strnlen(name) == 60, the string has no null terminator.
} dir_entry;

void ingest_opt(int argc, char* argv[]);
void print_help(char*);

// Client exposed: find a file given an image file pointer and a path. Returns the inode of that file if successful.
// iterates over directory files until the inode for the desired path is found.
inode find_file(FILE* img_fp, char* path);

// Given a sector number, a partition number, return a partition table entry. Sector number corresponds to beginning of any partition.
// then minls and minget can get a partition table entry and run this method again with sector number=lfirst if a subpartition was given
pte locate_pte(FILE* img_fp, uint8_t start_sec, uint8_t partition_num);

// given a sector number, return the superblock of a partition starting at that sector block
// with the superblock, you can find the root inode which (should) be a directory
superblock locate_sb(FILE* img_fp, uint8_t start_sec);



// inode 0 is located at (i_blocks + z_blocks + 3) * block_size gives you location of inode table. then you can index inode_table[1] to get the root inode

// verify if something is a directory or a file by checking mode. you take the file type mask and you do a bitwise and with the inodes mode. if it's a directory, you'll get 

// with the root inode, you find the 


#endif 
