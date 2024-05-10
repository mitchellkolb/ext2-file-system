
extern MINODE *root;
extern PROC proc[NPROC], *running;
extern int fd, dev, requests;

/*
//FROM MAIN
	//pathname is now oldNAME
	//parameter is now newNAME
int symlink(char *oldNAME, char *newNAME)
{
	printf("symlink\n");
// 3. ======================== HOW TO symlink ================================
// symlink oldNAME  newNAME    e.g. symlink /a/b/c /x/y/z

// ASSUME: oldNAME has <= 60 chars, inlcuding the ending NULL byte.


I got a the begining portion of this code from Gabe during his TA office hours. A lot of this was broken at the time and it smylinked correctly sometime after lvl1 but after we went to the 256 INODE size disk it sometimes has a rough time.

*/


int symlink(char *oldNAME, char *newNAME) {
    //oino and pino values of the parent directory and the newly created symlink file respectively. omip and pmip are pointers to the MINODE structs of the old and parent directories
    int oino, pino;
    MINODE *omip, *pmip;
    char parent[64], child[64], buf[BLKSIZE], *outArray[64];
    int n;

    // tokenize newNAME into parent and child name
    strcpy(buf, newNAME);
    n = tokenize(buf, outArray, "/");
    strcpy(parent, "");
    strcpy(child, "");
    if (n > 0) {
        strcpy(parent, outArray[0]);
    }
    if (n > 1) {
        strcpy(child, outArray[1]);
    }

    // get inumber of old file
    oino = path2inode(oldNAME);
    omip = iget(dev, oino);

// (1). verify oldNAME exists (either a DIR or a REG file)
    // check if old file exists and is a regular file
    if (!omip || !S_ISREG(omip->INODE.i_mode)) {
        iput(omip);
        printf("Error: %s does not exist or is not a regular file\n", oldNAME);
        return -1;
    }

    // get inumber of parent directory
    pino = path2inode(parent);
    pmip = iget(dev, pino);

    // check if parent directory exists and is a directory
    if (!pmip || !S_ISDIR(pmip->INODE.i_mode)) {
        iput(pmip);
        printf("Error: %s does not exist or is not a directory\n", parent);
        return -1;
    }

// (2). creat a FILE /x/y/z
    // create new file with name child in parent directory
    my_creat(pmip, child);
    int ino = path2inode(pmip);
    MINODE *mip = iget(dev, ino);

// (3). change /x/y/z's type to LNK (0120000)=(b1010.....)=0xA...
    // set new file type and inode mode
    mip->INODE.i_mode = 0120000; // set symlink file type
    mip->INODE.i_size = strlen(oldNAME); // set symlink size

// (4). write the string oldNAME into the i_block[ ], which has room for 60 chars.
    memcpy(mip->INODE.i_block, oldNAME, strlen(oldNAME)); // copy old file name to symlink i_block
    mip->modified = 1;

    // update parent directory
    enter_child(pmip, ino, child);

// (5). write the INODE of /x/y/z back to disk.
    // decrement old file's link count and mark as modified
    omip->INODE.i_links_count--;
    omip->modified = 1;
    iput(omip);

    // release MINODEs
    iput(pmip);
    iput(mip);

    return 0;
}
