// main.c file
#include "type.h"
//test
// block number = ((ino - 1) % inodes_per_block) + inode_start_block
/********** globals **************/
PROC   proc[NPROC];
PROC   *running;

MINODE minode[NMINODE];   // in memory INODES
MINODE *freeList;         // free minodes list
MINODE *cacheList;        // cached minodes list

MINODE *root;             // root minode pointer

OFT    oft[NOFT];         // for level-2 only

char gline[256];          // global line hold token strings of pathname
char *name[64];           // token string pointers
int  n;                   // number of token strings                    

int ninodes, nblocks;     // ninodes, nblocks from SUPER block
int bmap, imap, inodes_start, iblk;  // bitmap, inodes block numbers
int inode_size;

int  fd, dev;                                 //fd = opened vidsk for read(lab5 never used)   dev = set to open disk image in read only and to print device number(lab5 line 231)
char cmd[16], pathname[128], pathname2[128], parameter[128];  //used to store cmd, pathname, parameter labels. Pathname2 is used for link/syslink
int  requests, hits, misses;                          //Used for the hits command
SUPER *sp;  // Pointer to superblock
GD *gp;  // 

// start up files
#include "util.c"
#include "cd_ls_pwd.c"
#include "dalloc.c"
#include "mkdir_creat.c"
#include "rmdir.c"
#include "link_unlink.c"
#include "symlink.c"

#include "open_close_lseek.c"
#include "read.c"
#include "write.c"
#include "cat_cp.c"
#include "head_tail.c"


int init()
{
  int i, j;
  // initialize minodes into a freeList
  for (i=0; i<NMINODE; i++){
    MINODE *mip = &minode[i];
    mip->dev = mip->ino = 0;
    mip->id = i;
    mip->shareCount = 0;
    mip->next = &minode[i+1];
  }
  minode[NMINODE-1].next = 0;
  freeList = &minode[0];       // free minodes list

  cacheList = 0;               // cacheList = 0

  for (i=0; i<NOFT; i++)
    oft[i].shareCount = 0;     // all oft are FREE
 
  for (i=0; i<NPROC; i++){     // initialize procs
     PROC *p = &proc[i];    
     p->uid = p->gid = i;      // uid=0 for SUPER user
     p->pid = i+1;             // pid = 1,2,..., NPROC-1

     for (j=0; j<NFD; j++)
       p->fd[j] = 0;           // open file descritors are 0
  }
  
  running = &proc[0];          // P0 is now a running process
  requests = hits = 0;         // for hit_ratio of minodes cache
}


//char *disk = "./disk2";//256 inode size
//char *disk = "./diskimage";//128 inode size
char *disk = "./testdisk1";

int main(int argc, char *argv[ ]) 
{
  char line[128];
  char buf[BLKSIZE];

  //Initializes the minodes, cachelist, procs, opens file desc
  init();
  
  //Open disk image in read only
  fd = dev = open(disk, O_RDWR);;
  if (dev < 0) {
      printf("Failed to open diskimage file\n");
      exit(1);
  }

  // get super block of dev
  get_block(dev, 1, buf);
  sp = (SUPER *)buf;  // you should check s_magic for EXT2 FSs

  // Check if super block magic number matches ext2 file system
  // check EXT2 FS magic number:
  printf("Dev = %d\n", dev);
  printf("Check: Superblock Magic = 0x%x", sp->s_magic);
  if (sp->s_magic != 0xEF53){
    printf("NOT an EXT2 FS\n");
    exit(1);
  }
  else {
    printf("  OK\n");
  }
  printf("ninodes=%d   nblcks=%d   inode_size=%d\n", sp->s_inodes_count, sp->s_blocks_count, sp->s_inode_size);

  
  ninodes = sp->s_inodes_count;
  nblocks = sp->s_blocks_count;
  inode_size = sp->s_inode_size; // add this line to get the inode_size

  // calculate the sizeof(INODE) using the formula sizeof(INODE) = 128 << inode_size
  //printf("size%d\n", inode_size);//prints out 256
  size_t sizeof_inode = 128 << inode_size; //This line produces a number the size of 1.15792 * 10^77 its basically 2^256
  //printf("ninodes=%d  nblocks=%d  inode_size=%d  sizeof(INODE)=%lu\n", ninodes, nblocks, inode_size, sizeof_inode);

  get_block(dev, 2, buf);
  gp = (GD *)buf;

  bmap = gp->bg_block_bitmap;
  imap = gp->bg_inode_bitmap;
  iblk = inodes_start = gp->bg_inode_table;

  printf("bmap=%d  imap=%d  iblk=%d\n", bmap, imap, iblk);

  root = iget(dev, 2);
  running->cwd = iget(dev, 2); //Setting cwd to root at start


  printf("root shareCount = %d\n", root->shareCount);


  //printf("root shareCount = %d\n", root->shareCount);

  /*
  tokenize("/a/b/c");
  printf("this is name[0]:%s\n", name[0]);
  printf("enter a key to continue: ");
  getchar();
  */

  while(1){
     printf("P%d running\n", running->pid);
     pathname[0] = parameter[0] = 0;

     printf("Hits: %d\nmisses: %d\n", hits, misses);
      
     printf("\nenter command [cd|ls|pwd|mkdir|creat|rmdir]\n\t      [link|unlink|symlink|show|hits]\n\t      [open|close|lseek|read|write]\n\t      [pfd|cat|head|tail|cp|mv|exit] : ");
     fgets(line, 128, stdin);
     line[strlen(line)-1] = 0;    // kill \n at end

     if (line[0]==0)
        continue;

     sscanf(line, "%s %s %64c", cmd, pathname, parameter);
     printf("pathname=%s parameter=%s\n", pathname, parameter);
      
     if (strcmp(cmd, "ls")==0)
	     ls(pathname);
     if (strcmp(cmd, "cd")==0)
	     cd(pathname);
     if (strcmp(cmd, "pwd")==0)
       pwd(running->cwd);

     if (strcmp(cmd, "creat")==0)
       creat_file(pathname); 
     if (strcmp(cmd, "mkdir")==0)
       make_dir(pathname);
     if (strcmp(cmd, "rmdir")==0)
       rm_dir(pathname);
     if (strcmp(cmd, "link")==0)
        link(pathname, parameter);
     if (strcmp(cmd, "unlink")==0)
       unlink(pathname);
     if (strcmp(cmd, "symlink")==0)
       symlink(pathname, parameter);
     if (strcmp(cmd, "open")==0)
       open_file(pathname, parameter);
     if (strcmp(cmd, "pfd")==0)
       pfd();
     if (strcmp(cmd, "close")==0)
       close_file(pathname);
     if (strcmp(cmd, "read")==0)
       read_file();
     if (strcmp(cmd, "cat")==0)
       cat(pathname);
     if (strcmp(cmd, "head")==0)
        head(pathname);
     if (strcmp(cmd, "tail")==0)
        tail(pathname);


     if (strcmp(cmd, "write")==0)
       write_file();
     if (strcmp(cmd, "mv")==0)
       mv(pathname, parameter);
     if (strcmp(cmd, "cp")==0)
       cp(pathname, parameter);


     if (strcmp(cmd, "show")==0)
	     show_dir(pathname);
     if (strcmp(cmd, "hits")==0)
	     hit_ratio();
     if (strcmp(cmd, "exit")==0)
	     quit();
  }
}


void show_dir(char pathname[])
{

  //Initializing variables needed to step through entiries in a DIR data block
  MINODE *mip = malloc(sizeof(MINODE));
  mip = running->cwd;

  char sbuf[BLKSIZE], temp[256];
  DIR *dp;
  char *cp;

  printf("i_block[0] = %d\n", mip->INODE.i_block[0]);

  // ASSUME only one data block i_block[0]
  get_block(dev, mip->INODE.i_block[0], sbuf);

  dp = (DIR *)sbuf;
  cp = sbuf;

  //This is just to make the output look nicer like in the example output
  printf("   i_number  rec_len  name_len  name\n");

  while(cp < sbuf + BLKSIZE){
    strncpy(temp, dp->name, dp->name_len);
    temp[dp->name_len] = 0;

    printf("   %5d %8d %8d      %-7s\n", dp->inode, dp->rec_len, dp->name_len, temp);

    cp += dp->rec_len;
    dp = (DIR *)cp;
  }
}



void hit_ratio() {
  MINODE *mip = cacheList;
  int hit_count = 0, miss_count = 0;

  while (mip != NULL) {
    if (mip->shareCount > 0) {
      hit_count++;
    } else {
      miss_count++;
    }

    mip = mip->next;
  }

  int total_count = hit_count + miss_count;
  float hit_ratio = 0.0;
  if (total_count > 0) {
    hit_ratio = ((float)hit_count / (float)total_count) * 100.0;
  }

  printf("cacheList=");
  mip = cacheList;
  while (mip != NULL) {
    printf("c%d[%d %d]->", mip->cacheCount, mip->dev, mip->ino);
    mip = mip->next;
  }
  printf("NULL\n");

  printf("requests=%d hits=%d hit_ratio=%.0f%%\n", total_count, hit_count, hit_ratio); 
}

int quit()
{
   MINODE *mip = cacheList;
   while(mip){
     if (mip->shareCount){
        mip->shareCount = 1;
        iput(mip);    // write INODE back if modified
     }
     mip = mip->next;
   }
   exit(0);
}


/*

# mkdisk sh script

sudo dd if=/dev/zero of=diskimage bs=1024 count=4096

sudo mke2fs -b 1024 -I 128 diskimage 4096    # INODE size=256 bytes 

sudo mount diskimage /mnt

(cd /mnt; sudo rmdir lost+found; sudo mkdir dir1 dir2;
                                 sudo mkdir dir1/dir3 dir2/dir4;
                                 sudo touch file1 file2)

sudo umount /mnt


show diskimage


this is a tiny file
with only a few lines of text
happy testing


#!/bin/bash

# Create disk image
sudo dd if=/dev/zero of=testdisk1 bs=1024 count=4096
sudo mke2fs -b 1024 -I 128 testdisk1 4096    # INODE size=256 bytes 

# Mount disk image
sudo mount testdisk1 /mnt

# Create directories and files
(cd /mnt; sudo rmdir lost+found; sudo mkdir dir1 dir2;
                                 sudo mkdir dir1/dir3 dir2/dir4;
                                 sudo touch file1 file2 test)

# Add text to test file
sudo sh -c "echo 'this is a tiny file' > /mnt/test"
sudo sh -c "echo 'with only a few lines of text' >> /mnt/test"
sudo sh -c "echo 'happy testing' >> /mnt/test"

# Unmount disk image
sudo umount /mnt

# Display disk image contents
sudo fdisk -l testdisk1

*/