// util.c file
/*********** globals in main.c ***********/
extern PROC   proc[NPROC];
extern PROC   *running;

extern MINODE minode[NMINODE];   // minodes
extern MINODE *freeList;         // free minodes list
extern MINODE *cacheList;        // cacheCount minodes list

extern MINODE *root;

extern OFT    oft[NOFT];

extern char gline[256];   // global line hold token strings of pathname
extern char *name[64];    // token string pointers
extern int  n;            // number of token strings                    

extern int ninodes, nblocks;
extern int bmap, imap, inodes_start, iblk;  // bitmap, inodes block numbers

extern int  fd, dev;
extern char cmd[16], pathname[128], parameter[128];
extern int  requests, hits, inodes_start, misses;

/**************** util.c file **************/

int get_block(int dev, int blk, char buf[ ])
{
  lseek(dev, blk*BLKSIZE, SEEK_SET);
  int n = read(fd, buf, BLKSIZE);
  return n;
}

int put_block(int dev, int blk, char buf[ ])
{
  lseek(dev, blk*BLKSIZE, SEEK_SET);
  int n = write(fd, buf, BLKSIZE);
  return n; 
}       


//This C function takes in a string, delimiter and an array of strings as input parameters. The string is then tokenized (split) based on the specified delimiter and the resulting tokens are stored in the array of strings.
int tokenize(char *string, char **outArray[64], char *delim)
{
  //The function starts by initializing a counter variable n to zero and allocating memory for a backup copy of the input string using malloc() and strcpy().
  int n = 0;
  char *temp, *backup;
  backup = (char*) malloc(sizeof(char) * strlen(string));
  strcpy(backup, string);
  
  //Then, it enters a loop where the string is tokenized using the strtok() function. The loop continues until all tokens have been extracted from the string. Each extracted token is stored in the array of strings outArray at the current index n. The next index in the array is set to NULL to indicate the end of the string.
  for (temp = strtok(backup, delim); temp != NULL; temp = strtok(NULL, delim)) {
    outArray[n] = temp;
    outArray[n+1] = NULL;
    n++;
  }

  //After the loop finishes, if at least one token was extracted, n is decremented by one since the last index in the array should be NULL and not a token.
  if (n > 0) {
    n--;
  }
  
  //Finally, the function returns the number of tokens extracted.
  return n;
}



MINODE *dequeue(MINODE **queue)
{
  MINODE *temp = *queue;
  
  if (*queue == NULL) {
    // queue is empty
    return NULL;
  } else if ((*queue)->next == NULL) {
    // queue has only one element
    *queue = NULL;
  } else {
    // queue has more than one element
    *queue = (*queue)->next;
  }

  temp->next = NULL;
  return temp;
}

// iget returns a pointer to the in-memory INODE of (dev,ino). 
// The returned inode is unique which means that only one copy of the INODE exists in memory.
// Also the minode is locked for exlvlusive uses until it is either released or unlocked.
MINODE *iget(int dev, int ino)
{
  MINODE *mip;
  int i, blk, offset;
  char buf[BLKSIZE];
  INODE *ip;

  // 1. search cache for a minode with dev, ino
  for (mip = cacheList; mip != 0; mip = mip->next) {
    if (mip->dev == dev && mip->ino == ino) {
      mip->shareCount++;     // increment reference count
      mip->cacheCount++;
      hits++;              // increment cache hits counter
      return mip;          // return found minode
    }
  }

  misses++;

  // 2. get a free minode (shareCount=1, dev=dev, ino=ino) from freeList
  mip = freeList;
  if (mip == 0) {
    printf("PANIC: no more free minodes\n");
    return 0;
  }
  freeList = freeList->next;    // remove minode from freeList
  mip->shareCount = 1;            // set reference count to 1
  mip->dev = dev;               // set dev to dev
  mip->ino = ino;               // set ino to ino
  mip->cacheCount = 1;
  mip->modified = 0;

  // // 3. get INODE of (dev, ino) into buf[ ]
  // int ifactor = 256/128;
  // blk = (ino - 1) / ifactor + iblk;
  // offset = (ino - 1) % ifactor;
  // get_block(dev, blk, buf);
  // ip = (INODE *)buf + offset * ifactor;
  
  //Old mailman algo
  //Going to be used for final presentation
  blk = (ino - 1) / 8 + inodes_start;
  offset = (ino - 1) % 8;
  get_block(dev, blk, buf); 
  ip = (INODE *)buf + offset;


  // 4. copy INODE into minode.INODE
  mip->INODE = *ip;

  // 6. add new minode to cacheList
  mip->next = cacheList;
  cacheList = mip;

  // 7. return pointer to the in-memory INODE in the minode
  return mip;
}



//This function releases and unlocks a minode pointed by ,ip. If the process is the last one to use the minode (shareCount = 0), the INODE is written back to disk if it is modified
int iput(MINODE *mip)  // release a mip
{
  if (mip == NULL) {
    return 0;
  }

  mip->shareCount--;

  if (mip->shareCount > 0) {
    return 0;
  }

  if(mip->modified == 0)
  {
    return -1;
  }

  // write mip's INODE back to disk
  char buf[BLKSIZE];

  // int ifactor = 256/128;
  // int blk = (mip->ino - 1) / ifactor + iblk;
  // int offset = (mip->ino - 1) % ifactor;
  // get_block(mip->dev, blk, buf);
  // INODE *ip = (INODE *)buf + offset;

  //Old mailman algo
  //Going to be used for final presentation
  int blk = (mip->ino - 1) / (BLKSIZE / sizeof(INODE)) + iblk;
  int offset = (mip->ino - 1) % (BLKSIZE / sizeof(INODE));
  get_block(dev, blk, buf); 
  INODE *ip = (INODE *)buf + offset;



  *ip = mip->INODE;
  put_block(mip->dev, blk, buf);

  // remove mip from cacheList
  MINODE *prev = NULL;
  MINODE *cur = cacheList;
  while (cur && cur != mip) {
    prev = cur;
    cur = cur->next;
  }
  if (cur == NULL) {
    // mip not in cacheList, should not happen
    printf("ERROR: mip not in cacheList\n");
    return -1;
  }
  if (prev == NULL) {
    cacheList = mip->next;
  } else {
    prev->next = mip->next;
  }

  // put mip back to freeList
  mip->next = freeList;
  freeList = mip;

  return 1;
}


int search(MINODE *mip, char *name)
{
  int i;
  char *cp;
  DIR *dp;
  char buf[BLKSIZE];

  // search direct blocks
  for (i = 0; i < 12 && mip->INODE.i_block[i]; i++) {
    get_block(mip->dev, mip->INODE.i_block[i], buf);

    dp = (DIR *)buf;
    cp = buf;

    while (cp < buf + BLKSIZE) {
      if (strncmp(dp->name, name, dp->name_len) == 0) {
        // found name, return inode number
        return dp->inode;
      }
      cp += dp->rec_len;
      dp = (DIR *)cp;
    }
  }

  // search indirect blocks
  if (mip->INODE.i_block[12]) {
    int blk, *ip;

    get_block(dev, mip->INODE.i_block[12], buf);
    ip = (int *)buf;

    while (*ip) {
      get_block(dev, *ip, buf);

      dp = (DIR *)buf;
      cp = buf;

      while (cp < buf + BLKSIZE) {
        if (strncmp(dp->name, name, dp->name_len) == 0) {
          // found name, return inode number
          return dp->inode;
        }
        cp += dp->rec_len;
        dp = (DIR *)cp;
      }

      ip++;
    }
  }

  // name not found
  return 0;
}

// Returns a pointer to the inode for the given pathname, or NULL if the file is inaccessible.
// Searches over each component of pathname, checking if it is a file or directory.
// Returns the pointer to the inode.
MINODE *path2inode(char *pathname)
{
  MINODE *mip;
  int ino;

  // get the starting directory's ino
  if (pathname[0] == '/') {
    mip = iget(root->dev, 2);
  } else {
    mip = iget(running->cwd->dev, running->cwd->ino);
  }

  
  // tokenize the pathname and traverse the directory tree
  char *token;
  token = strtok(pathname, "/");
  while (token != NULL) {
    if (strcmp(token, ".") == 0) 
    {
      // do nothing, already at this directory
    } 
    else if (strcmp(token, "..") == 0) 
    {
      // move up one level in the directory tree
      ino = findino(mip, &ino);
      iput(mip);
      mip = iget(dev, ino);
    } 
    else //If we are here we know that what we want is not in the cwd and not in the above directory so we do a search and release the blocks if it exists
    {
      ino = search(mip, token);
      if (ino == 0) {
        iput(mip);
        return NULL;  // file/directory does not exist
      }
      iput(mip);
      mip = iget(dev, ino);
    }
    
    token = strtok(NULL, "/");
  }

  return mip;
}



// Searches for a given ino number in the Parent inode directory
// Returns name if ino is found
int findmyname(MINODE *pip, int myino, char myname[ ]) 
{
  /****************
  pip points to parent DIR minode: 
  search for myino;    // same as search(pip, name) but search by ino
  copy name string into myname[256]
  ******************/
  //(MINODE *mip, char *name)
  int i;
  char *cp;
  char temp[256];
  DIR *dp;
  char buf[BLKSIZE];
  MINODE *mip = pip;

  // search direct blocks
  for (i = 0; i < 12 && mip->INODE.i_block[i]; i++) {
    //Check for empty blocks
    if(mip->INODE.i_block[i] == 0)
    {
      return -2;
    }


    get_block(dev, mip->INODE.i_block[i], buf);

    dp = (DIR *)buf;
    cp = buf;

    while (cp < buf + BLKSIZE) {
      strncpy(temp, dp->name, dp->name_len);
      temp[dp->name_len] = 0;
      
      if(dp->inode == myino)
      {
        strncpy(myname, dp->name, dp->name_len);
        myname[dp->name_len] = 0;
        return 0;
      }
      
      cp += dp->rec_len;
      dp = (DIR *)cp;
    }
  }

  //Don't need to go through indirect blocks. Unlike search() this only checks direct blocks. The Dir should be in this level

  // name not copied over
  return -1;
}

// Finds the inode of the current dir and parent dir and returns the inode the parent dir indirectly
// Loops through each directory entry in block
// Checks if current directory name is "." --> updates ino with current directory inode number
// if current directory name  is ".." then we update ino with parent directory inode number
// Returns ino if we found the directory
int findino(MINODE *mip, int *myino)
{
    // Check that mip is a directory
    if (!S_ISDIR(mip->INODE.i_mode)) {
        printf("Error: inode is not a directory\n");
        return -1;
    }

    char buf[BLKSIZE];
    char name[256];

    // Get the inode number of the given directory
    *myino = mip->ino;

    // Read the first block of the directory
    get_block(mip->dev, mip->INODE.i_block[0], buf);
    char *cp = buf;
    DIR *dp = (DIR *)buf;

    // Loop through each directory entry in the block
    while (cp < buf + BLKSIZE) {
        // Get the name of the current directory entry
        strncpy(name, dp->name, dp->name_len);
        name[dp->name_len] = '\0';

        // Check if the current directory entry is the current directory "."
        if (strcmp(name, ".") == 0) {
            // Get the inode number of the current directory
            *myino = dp->inode;
        }

        // Check if the current directory entry is the parent directory ".."
        else if (strcmp(name, "..") == 0) {
            // Get the inode number of the parent directory
            return dp->inode;
        }

        // Move to the next directory entry
        cp += dp->rec_len;
        dp = (DIR *)cp;
    }

    return -1; // If we reach here, we didn't find ".."
}

// Returns the inode number of the given pathname
// Searches through directory entries pointed to by mip
// Searches entry for value given by name
int getino(MINODE *mip, char *name)
{
    // Buffers to hold block data
    char buf[1024], buf2[1024];

    // Track state. Result = -2 means we could not find an inode number
    int i, j = 1, inodenumber, result = -2, size = 0;

    // Load the first data block of the directory pointed by mip
    get_block(mip->dev, mip->INODE.i_block[0], buf2);

    // Directory pointer
    DIR *dp;
    dp = (DIR *)buf2;
    char *cp = buf2;

    // cp = current position
    // While we haven't reached the end of the buffer
    // we add 1024 to point to the end of the buffer.
    while (cp < buf2 + 1024) {
        // A temporary buffer for holding the name of each directory entry
        char temp[100];
        strncpy(temp, dp->name, dp->name_len);

        // Terminate the temporary buffer with a null character
        // ^^ we do this to ensure strcmp works
        temp[dp->name_len] = 0;
        if (strcmp(name, temp) == 0) {
            inodenumber = dp->inode;
            iput(mip);
            return inodenumber;
        }

        // Clear temp buffer
        memset(temp, 0, 100);
        cp += dp->rec_len;
        dp = (DIR *)cp;
    }

    // Release MINODE
    iput(mip);
    return result;
}

// Allocate a new free block on the given device and return its block number
// Bitmap tracks which blocks are in use and whats free to use
// Iterates through blocks in the bitmap
int balloc(int dev) {
    // buffer to hold block bitmap
    char buf[BLKSIZE];
    int block_no, byte_offset, bit_offset;

    // Read block bitmap into buffer
    get_block(dev, bmap, buf);

    // Iterate over all blocks in the bitmap
    for (block_no = 0; block_no < nblocks; block_no++) {
        // Calculate byte and bit offset for current block
        byte_offset = block_no / 8;
        bit_offset = block_no % 8;

        // Check if the bit corresponding to the current block is 0
        if ((buf[byte_offset] & (1 << bit_offset)) == 0) { // & is bitwise 
            /* Mark the bit as 1 to indicate the block is now in use */
            buf[byte_offset] |= (1 << bit_offset);// |= is the bitwise or

            /* Write updated bitmap back to disk */
            put_block(dev, bmap, buf);

            /* Return the block number of the newly allocated block */
            return block_no + 1;
        }
    }

    /* If no free blocks were found, print an error message and return 0 */
    printf("balloc(): no more free blocks\n");
    return 0;
}


int decFreeInodes(int dev)
{
  char buf[BLKSIZE];

  // dec free inodes count by 1 in SUPER and GD
  get_block(dev, 1, buf);
  sp = (SUPER *)buf;
  sp->s_free_inodes_count--;
  put_block(dev, 1, buf);

  get_block(dev, 2, buf);
  gp = (GD *)buf;
  gp->bg_free_inodes_count--;
  put_block(dev, 2, buf);
}

int ialloc(int dev)  // allocate an inode number from imap block
{
  int  i;
  char buf[BLKSIZE];

  // read inode_bitmap block
  get_block(dev, imap, buf);

  for (i=0; i < ninodes; i++){
    if (tst_bit(buf, i)==0){
       set_bit(buf,i);
       put_block(dev, imap, buf);
    
       decFreeInodes(dev);

       printf("ialloc : ino=%d\n", i+1);    
       return i+1;
    }
  }
  return 0;
}


int tst_bit(char *buf, int bit) {
    // Calculate which byte the bit is in
    int byte_index = bit / 8;

    // Calculate which bit within the byte the bit is
    int bit_index = bit % 8;

    // Calculate the mask to check against
    // Shift bit to the left
    char mask = 1 << bit_index;

    // Check whether the bit is set by ANDing the byte with the mask
    if (buf[byte_index] & mask) {
        return 1;
    } else {
        return 0;
    }
}

void set_bit(char *buf, int bit) {
    // Calculate which byte the bit is in
    int byte_index = bit / 8;

    // Calculate which bit within the byte the bit is
    int bit_index = bit % 8;

    // Calculate the mask to set the bit with
    // Shift bit to the left
    char mask = 1 << bit_index;

    // Set the bit by ORing the byte with the mask
    // AND operation sets the target bit to 0 while leaving all other bits unchanged, since 0 & 1 is always 0 and 0 & 0 is always
    buf[byte_index] |= mask;
}

void clr_bit(char *buf, int bit) {
    // Where the bit is to be cleared
    int byteIndex = bit / 8;

    // bit offset of the bit to be cleared within the byte
    int bitOffset = bit % 8;

    // creates a bitmask with all bits set to 1, except for the target bit to be cleared, which is set to 0
    char mask = ~(1 << bitOffset);

    // Clears bit by performing bitwise AND between byte containing the bit and the bitmask

    buf[byteIndex] = buf[byteIndex] & mask;
}

int enter_child(MINODE *pip, int myino, char *myname)
{
    int i, need_len, ideal_len, remain, new_block;
    char *cp, buf[BLKSIZE];
    DIR *dp;
    INODE *pip_inode = &pip->INODE;

    need_len = 4 * ((8 + strlen(myname) + 3) / 4);

    for (i = 0; i < 12; i++) {  // assume: only 12 direct blocks
        if (pip_inode->i_block[i] == 0) {
            break;
        }

        get_block(dev, pip_inode->i_block[i], buf);
        cp = buf;
        dp = (DIR *)buf;

        while (cp < buf + BLKSIZE) {
            // Compute the ideal length for this directory entry
            ideal_len = 4 * ((8 + dp->name_len + 3) / 4);

            // Compute the remaining space in this directory entry
            remain = dp->rec_len - ideal_len;

            if (remain >= need_len) {
                // Found enough space for the new entry
                dp->rec_len = ideal_len;

                // Move cp to the end of this directory entry
                cp += dp->rec_len;
                dp = (DIR *)cp;

                // Create the new entry
                dp->inode = myino;
                dp->rec_len = remain;
                dp->name_len = strlen(myname);
                dp->file_type = EXT2_FT_DIR;
                strcpy(dp->name, myname);

                // Write the block back to disk
                put_block(dev, pip_inode->i_block[i], buf);

                // Return success
                return 0;
            }

            // Move cp to the next directory entry
            cp += dp->rec_len;
            dp = (DIR *)cp;
        }
    }

    // If we reach here, there is no space in the existing data blocks
    // Allocate a new data block
    new_block = balloc(dev);

    if (new_block == 0) {
        printf("Error: no free blocks\n");
        return -1;
    }

    // Increment the parent's size by BLKSIZE
    pip_inode->i_size += BLKSIZE;

    // Enter the new entry as the first entry in the new data block
    get_block(dev, new_block, buf);
    dp = (DIR *)buf;
    dp->inode = myino;
    dp->rec_len = BLKSIZE;
    dp->name_len = strlen(myname);
    dp->file_type = EXT2_FT_DIR;
    strcpy(dp->name, myname);
    put_block(dev, new_block, buf);

    // Add the new block to the parent's i_block array
    for (i = 0; i < 12; i++) {
        if (pip_inode->i_block[i] == 0) {
            pip_inode->i_block[i] = new_block;
            break;
        }
    }

    // Write the parent's inode back to disk
    pip->modified = 1;
    iput(pip);

    // Return success
    return 0;
}

int isEmpty(MINODE *mip)
{
  char buf[BLKSIZE], namebuf[256], *cp;
  DIR *dp;

  // If link count is 2, check each direct block for entries
  if(mip->INODE.i_links_count == 2)//any with the linkcount == 2 means we have . or .. files so we must a have directory
  {
    // traverse each block
    for(int i = 0; i <= 11; i++)
    {
      // If direct block exists
      if(mip->INODE.i_block[i])
      {
        // Check each directory entry in the block
        get_block(mip->dev, mip->INODE.i_block[i], buf);
        cp = buf;
        dp = (DIR *)buf;
        while(cp < &buf[BLKSIZE])//&buf[BLKSIZE] is the same as eariler when we did cp < buf + BLKSIZE
        {
          strncpy(namebuf, dp->name, dp->name_len);
          namebuf[dp->name_len] = 0;

          // If not . or .., directory is not empty
          if(strcmp(namebuf, ".") && strcmp(namebuf, "..")) {
            return 0;
          }

          cp += dp->rec_len;
          dp = (DIR *)cp;
        }
      }
    }
    // If directory is empty, return 1
    return 1;
  }
  // If link count is less than 2, directory is not valid
  return -1;
}


int truncate(MINODE* mip)
{
    // Deallocate direct blocks
    for (int i = 0; i < 12; i++) {
        // if the block exists then deallocate it, set the block to 0
        if (mip->INODE.i_block[i]) {
            bdalloc(mip->dev, mip->INODE.i_block[i]);
            mip->INODE.i_block[i] = 0;
        }
    }

    // Deallocate indirect block
    if (mip->INODE.i_block[12]) {
        char buf[BLKSIZE];
        int *indirect = (int *)buf;
        get_block(mip->dev, mip->INODE.i_block[12], buf);

        // Divide BLKSIZE / sizeof(int) to obtain how many block pointers we can store 
        for (int i = 0; i < BLKSIZE/sizeof(int); i++) {
            if (indirect[i]) {
                bdalloc(mip->dev, indirect[i]);
                indirect[i] = 0;
            }
        }

        bdalloc(mip->dev, mip->INODE.i_block[12]);
        mip->INODE.i_block[12] = 0;
    }

    // Deallocate double indirect block
    if (mip->INODE.i_block[13]) {
        char buf[BLKSIZE];
        int *indirect1 = (int *)buf;
        get_block(mip->dev, mip->INODE.i_block[13], buf);

        for (int i = 0; i < BLKSIZE/sizeof(int); i++) {
            if (indirect1[i]) {
                char buf2[BLKSIZE];
                int *indirect2 = (int *)buf2;
                get_block(mip->dev, indirect1[i], buf2);
                for (int j = 0; j < BLKSIZE/sizeof(int); j++) {
                    if (indirect2[j]) {
                        bdalloc(mip->dev, indirect2[j]);
                        indirect2[j] = 0;
                    }
                }
                bdalloc(mip->dev, indirect1[i]);
                indirect1[i] = 0;
            }
        }

        bdalloc(mip->dev, mip->INODE.i_block[13]);
        mip->INODE.i_block[13] = 0;
    }

    mip->INODE.i_size = 0;
    mip->INODE.i_mtime = time(0L);
    mip->modified = 1;
    return 0;
}
