

extern MINODE *root;
extern PROC proc[NPROC], *running;
extern int fd, dev, requests;


int rm_dir(char* pathname) {
  // Step 1: Get the MINODE and INODE for the directory specified by pathname
  MINODE *mip = path2inode(pathname);
  INODE *ip = &mip->INODE;

  // Step 2: Verify that the pathname is a valid directory
  if (!S_ISDIR(ip->i_mode)) {
    printf("%s is not a directory\n", pathname);
    iput(mip);
    return -1;
  }

  // Step 3: Check if the directory is busy
  if (mip->shareCount != 1) {
    printf("%s is busy\n", pathname);
    iput(mip);
    return -1;
  }

  // Step 4: Check if the directory is empty besides "." and ".."
  if (ip->i_links_count > 2) {
    // Directory is not empty
    iput(mip);
  } else {
    // Directory is empty
    if (!isEmpty(mip)) {
      printf("%s was not empty\n", pathname);
      return -1;
    }
  }

  // Step 5: Deallocate the data blocks and inode for the directory
  for (int i = 0; i < 12; i++) {
    if (mip->INODE.i_block[i] == 0) {
      continue;
    }

    // Deallocate data block
    // marks the block as free in the block bitmap
    // updates the superblock and group descriptor to reflect the change
    // writes the updated bitmap block to disk.
    bdalloc(mip->dev, mip->INODE.i_block[i]);
  }

  // Deallocate inode directory
  // marks inode as free in the inode bitmap
  // updates the superblock and group descriptor to reflect the change
  // writes the updated bitmap block to disk.
  idalloc(mip->dev, mip->ino);
  iput(mip);

  // Step 6: Remove the directory's entry from its parent directory
  char parent_pathname[128], child_name[128];
  strcpy(parent_pathname, pathname);
  strcpy(child_name, pathname);

  // Get the parent directory's MINODE
  char* parent = dirname(parent_pathname);

  // Find parent ino
  int parent_ino = findino(mip, &mip->ino);
  MINODE *pip = iget(mip->dev, parent_ino);

  // Get the child directory's name and check if it is special
  char *name = basename(child_name);

  if (strcmp(name, "..") == 0) {
    printf("Cannot remove. Permission denied.\n");
    return -1;
  }
  if (strcmp(name, ".") == 0) {
    printf("Cannot remove. Permission denied.\n");
    return -1;
  }
  if (strcmp(name, "/") == 0) {
    printf("Cannot remove. Permission denied.\n");
    return -1;
  }

  // Remove the child directory's entry from the parent directory
  rm_child(pip, name);
  pip->INODE.i_links_count--; // Decrement parent directory's link count
  pip->INODE.i_atime = pip->INODE.i_mtime = time(0L); // Update parent directory's time fields
  pip->modified = 1; // Mark parent directory as modified
  iput(pip); // Cleanup

  return 0;
}

int rm_child(MINODE *parent, char *my_name) {
  char buf[BLKSIZE], *cp;
  DIR *dp, *prev_dp = NULL;
  int i;

  // search parent inode's data blocks for the entry of my_name
  for (i = 0; i <= 11 && parent->INODE.i_block[i]; i++) {
    get_block(parent->dev, parent->INODE.i_block[i], buf);
    dp = (DIR *) buf;
    cp = buf;

    while (cp < &buf[BLKSIZE]) {
      // found my_name? then we must erase it!
      if (!strcmp(dp->name, my_name)) {
        // if not first entry in data block
        if (prev_dp) {
          prev_dp->rec_len += dp->rec_len;
        } else {
          // if first entry in a data block
          parent->INODE.i_block[i] = 0;
          parent->INODE.i_size -= BLKSIZE;
          bdalloc(parent->dev, parent->INODE.i_block[i]);
        }

        put_block(parent->dev, parent->INODE.i_block[i], buf);
        parent->modified = 1;
        return 0;
      }

      prev_dp = dp;
      cp += dp->rec_len;
      dp = (DIR *) cp;
    }
  }

  return 1;
}

