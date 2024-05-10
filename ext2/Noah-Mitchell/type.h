/***** type.h file for CS360 Project *****/

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <time.h>
#include <string.h>
#include <libgen.h>
#include <sys/stat.h>

#include <ext2fs/ext2_fs.h>

// define shorter TYPES, save typing efforts
typedef struct ext2_super_block SUPER;
typedef struct ext2_group_desc  GD;
typedef struct ext2_inode       INODE;
typedef struct ext2_dir_entry_2 DIR; 

#define BLKSIZE           1024 

#define NPROC                2
#define NMINODE             64
#define NFD                  8
#define NOFT                32

// In-memory inodes structure
typedef struct minode{		
  INODE INODE;            // disk INODE
  int   dev, ino;
  int   cacheCount;       // minode in cache count
  int   shareCount;       // number of users on this minode
  int   modified;         // modified while in memory
  int   id;               // index ID
  struct minode *next;    // pointer to next minode

}MINODE;

// Open File Table
typedef struct oft{
  int   mode;
  int   shareCount;
  struct minode *inodeptr;
  long  offset;
  struct minode *minodeptr;
} OFT;


// PROC structure
typedef struct proc{
  int   uid;            // uid = 0 or nonzero
  int   gid;            // group ID = uid
  int   pid;            // pid = 1 or 2
  struct minode *cwd;   // CWD pointer
  OFT   *fd[NFD];       // file descriptor array
} PROC;




      
