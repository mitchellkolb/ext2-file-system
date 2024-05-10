

extern MINODE *root;
extern PROC proc[NPROC], *running;
extern int fd, dev, requests;


int make_dir(char *pathname) {
    // check if pathname is provided
    if (!pathname[0]) {
        printf("mkdir: missing operand\n");
        return -1;
    }

    MINODE *pip, *mip;
    char pathcpy1[256], pathcpy2[256];

    strcpy(pathcpy1, pathname);
    strcpy(pathcpy2, pathname);

    //dir and basename destroy strings
    char *parent = dirname(pathcpy1);
    char *child = basename(pathcpy2);

    // get MINODE of parent
    pip = path2inode(parent);

    if (!pip) {
        printf("mkdir: %s does not exist\n", parent);
        return -1;
    }

    // verify that parent is a directory and that the child does not already exist in the parent directory
    if (!S_ISDIR(pip->INODE.i_mode)) {
        printf("mkdir: %s is not a directory\n", parent);
        iput(pip); // release MINODE
        return -1;
    }

    if (search(pip, child)) {
        printf("mkdir: %s already exists in %s\n", child, parent);
        iput(pip); // release MINODE
        return -1;
    }

    printf("%s\n", child);

    // call mymkdir() to create the new directory
    // uses the child name as the DIR name
    mymkdir(pip, child);

    // increment the parent directory's link count, update its access time and modification time, and release its MINODE pointer
    pip->INODE.i_links_count++; // increment link count
    pip->INODE.i_atime = time(0L); // update access time
    pip->INODE.i_mtime = time(0L); // update modification time
    pip->INODE.i_ctime = time(0L); // update modification time
    pip->modified = 1; // mark MINODE as modified
    //pip->modified = 1;
    iput(pip);

    printf("mkdir: directory '%s' created successfully\n", pathname);
    
    return 0;
}

// Creates new directory given name and parent directory
// Creates a new inode and data block for directory
// Sets properties of the new inode
// Creates block containing "." and '..' entries
// Updates parent directory to include new directory
// Finally writes new inode and the parent inode to the disk
int mymkdir(MINODE *pip, char *name)
{
    int ino, bno, i;
    DIR* dp;
    MINODE *mip;
    char buf[BLKSIZE];

    // allocates an unused inode from the inode bitmap of a given device
    ino = ialloc(dev);

    // allocates an unused block from the block bitmap of a given device
    bno = balloc(dev);

    printf("ino: %d    bno: %d\n", ino, bno);

    mip = iget(dev, ino); // load the inode into memory

    printf("mip->ino = %d, mip->dev = %d\n", mip->ino, mip->dev);
    INODE *ip = &mip->INODE;

    // set the new inode's properties
    ip->i_mode = 0x41ED; // Can also be 040755 (DIR type / Permissions)
    ip->i_uid = running->uid; // owner uid
    ip->i_gid = running->gid; // group id
    ip->i_size = BLKSIZE; // size in bytes
    ip->i_links_count = 2; // initial links count
    ip->i_atime = mip->INODE.i_ctime = mip->INODE.i_mtime = time(0L); // set to current time
    ip->i_block[0] = bno; // new DIR has one data block

    for (i = 1; i < 13; i++) // set the rest of the blocks to 0
        ip->i_block[i] = 0;

    mip->modified = 1; // mark the inode as modified
    //mip->modified = 1;

    iput(mip); // write the new inode to disk

    // create data block for new DIR containing . and .. entries
    get_block(dev, bno, buf);    // Get block  containing new directory data
    dp = (DIR *)buf; 
    dp->inode = ino;   // Set inode number of new directory entry to inode number
    strncpy(dp->name, ".", 1);
    dp->name_len = 1;
    dp->rec_len = 12; // Set record entry to 12 bytes (enough for one entry)

    // Move directory pointer to end of current directory entry
    dp = (DIR *)((char *)dp + dp->rec_len);

    // Set the inode number of the ".." directory entry to the inode number of the parent directory
    dp->inode = pip->ino;
    dp->name_len = 2;
    strncpy(dp->name, "..", 2);

    // set record length to remaining space in the block
    dp->rec_len = BLKSIZE - 12;


    int bytes_written = put_block(dev, bno, buf);
    if (bytes_written < BLKSIZE) {
      fprintf(stderr, "put_block failed: only %d bytes written\n", bytes_written);
      perror("put_block");
    }

    // add the new directory to the parent directory
    enter_child(pip, ino, name);
    pip->INODE.i_links_count++;
    pip->INODE.i_atime = time(0L); // update parent's atime
    pip->modified = 1; // mark the parent's inode as modified
    //pip->modified = 1;
    iput(pip); // write the parent's inode to disk

    return 0;
}


int creat_file(char *pathname)
{
    // check if pathname is provided
    if (!pathname[0]) {
        printf("mkdir: missing operand\n");
        return -1;
    }

    MINODE *pip;
    char pathcpy1[256], pathcpy2[256];

    strcpy(pathcpy1, pathname);
    strcpy(pathcpy2, pathname);

    char *parent = dirname(pathcpy1); //dir and basename are distructive to strings
    char *child = basename(pathcpy2);

    // get MINODE of parent
    pip = path2inode(parent);
    if (!pip) {
        printf("mkdir: %s does not exist\n", parent);
        return -1;
    }

    // verify that parent is a directory and that the child does not already exist in the parent directory
    if (!S_ISDIR(pip->INODE.i_mode)) {
        printf("mkdir: %s is not a directory\n", parent);
        iput(pip); // release MINODE
        return -1;
    }

    if (search(pip, child)) {
        printf("mkdir: %s already exists in %s\n", child, parent);
        iput(pip); // release MINODE
        return -1;
    }


    my_creat(pip, child);

    // increment the parent directory's link count, update its access time and modification time, and release its MINODE pointer
    pip->INODE.i_atime = time(0L); // update access time
    pip->INODE.i_mtime = time(0L); // update modification time
    pip->INODE.i_ctime = time(0L); // update modification time
    pip->modified = 1; // mark MINODE as modified
    //pip->modified = 1;
    iput(pip);

    printf("creat: file '%s' created successfully\n", pathname);
}

int my_creat(MINODE *pip, char *name)
{

  int ino, bno, i;
  DIR* dp;
  MINODE *mip;
  char buf[BLKSIZE];

  ino = ialloc(dev); // allocate an inode and a disk block for new file
  bno = balloc(dev);

  printf("ino: %d    bno: %d\n", ino, bno);

  mip = iget(dev, ino); // load the inode into memory

  printf("mip->ino = %d, mip->dev = %d\n", mip->ino, mip->dev);
  INODE *ip = &mip->INODE;

  // set the new inode's properties
  ip->i_mode = 0x81A4; // Can also be 0100644 (File type / Permissions)
  ip->i_uid = running->uid; // owner uid
  ip->i_gid = running->gid; // group id
  ip->i_size = 0; // size in bytes
  ip->i_links_count = 1; // initial links count
  ip->i_blocks = 2;
  ip->i_atime = mip->INODE.i_ctime = mip->INODE.i_mtime = time(0L); // set to current time
  ip->i_block[0] = 0; // new DIR has one data block

  for (i = 1; i < 13; i++) // set the rest of the blocks to 0
      ip->i_block[i] = 0;


  mip->modified = 1; // mark the inode as modified
  //mip->modified = 1;

  iput(mip); // write the new inode to disk


  enter_child(pip, ino, name);


  return 0;

}
