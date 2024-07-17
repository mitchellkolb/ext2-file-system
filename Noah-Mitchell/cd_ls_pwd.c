
// cd_ls_pwd.c file

extern MINODE *root;
extern PROC proc[NPROC], *running;
extern int fd, dev, requests;

int cd(char* pathname)
{
  requests++;

  // check if pathname is provided
  if (!pathname[0]) {
    printf("cd: missing operand\n");
    return;
  }

  // get MINODE of pathname
  MINODE *mip = path2inode(pathname);

  if (!mip) {
    printf("cd: %s does not exist\n", pathname);
    return;
  }

  if (!S_ISDIR(mip->INODE.i_mode)) { // check if MINODE is a directory
    printf("cd: %s is not a directory\n", pathname);
    iput(mip); // release MINODE
    return;
  }

  // release old cwd MINODE and set cwd to new MINODE
  iput(running->cwd);
  running->cwd = mip;
}

int ls_file(MINODE *mip, char *name)
{
  char ftime[64];
  time_t ctime = mip->INODE.i_ctime;
  strftime(ftime, 64, "%b %d %H:%M:%S %Y", localtime(&ctime));

  if (S_ISDIR(mip->INODE.i_mode))
    printf("d");
  else
    printf("-");

  printf((mip->INODE.i_mode & S_IRUSR) ? "r" : "-");
  printf((mip->INODE.i_mode & S_IWUSR) ? "w" : "-");
  printf((mip->INODE.i_mode & S_IXUSR) ? "x" : "-");
  printf((mip->INODE.i_mode & S_IRGRP) ? "r" : "-");
  printf((mip->INODE.i_mode & S_IWGRP) ? "w" : "-");
  printf((mip->INODE.i_mode & S_IXGRP) ? "x" : "-");
  printf((mip->INODE.i_mode & S_IROTH) ? "r" : "-");
  printf((mip->INODE.i_mode & S_IWOTH) ? "w" : "-");
  printf((mip->INODE.i_mode & S_IXOTH) ? "x" : "-");
  printf(" %d %d %d %10d %s %s [%d %d]\n", mip->INODE.i_links_count, mip->INODE.i_uid, mip->INODE.i_gid, mip->INODE.i_size, ftime, name, mip->dev, mip->ino);
}


void ls_dir(MINODE *pip)
{
  char sbuf[BLKSIZE], ftime[64];
  DIR  *dp;
  char *cp;
  MINODE *child;

  for (int i = 0; i < 12 && pip->INODE.i_block[i]; i++) {
    get_block(dev, pip->INODE.i_block[i], sbuf);
    dp = (DIR *)sbuf;
    cp = sbuf;

    while (cp < sbuf + BLKSIZE){
      child = iget(dev, dp->inode);
      time_t ctime = child->INODE.i_ctime;
      strftime(ftime, 64, "%b %d %H:%M:%S %Y", localtime(&ctime));

      if (dp->file_type == EXT2_FT_DIR) {
        printf("d");
      }
      else {
        printf("-");
      }

      printf((child->INODE.i_mode & 0x0100) ? "r" : "-");
      printf((child->INODE.i_mode & 0x0080) ? "w" : "-");
      printf((child->INODE.i_mode & 0x0040) ? "x" : "-");
      printf((child->INODE.i_mode & 0x0020) ? "r" : "-");
      printf((child->INODE.i_mode & 0x0010) ? "w" : "-");
      printf((child->INODE.i_mode & 0x0008) ? "x" : "-");
      printf((child->INODE.i_mode & 0x0004) ? "r" : "-");
      printf((child->INODE.i_mode & 0x0002) ? "w" : "-");
      printf((child->INODE.i_mode & 0x0001) ? "x" : "-");
      printf(" %2d %2d %2d %10d %s %s\n", child->INODE.i_links_count, child->INODE.i_uid, child->INODE.i_gid, child->INODE.i_size, ftime, dp->name);

      cp += dp->rec_len;
      dp = (DIR *)cp;
      iput(child);
    }
  }
}



void printdir(INODE ind){
  char buf[BLKSIZE];
  get_block(dev, ind.i_block[0], buf);

  DIR *dp = (DIR*)buf;
  char *cp = buf;

  while (cp < buf + BLKSIZE){
    char name[EXT2_NAME_LEN + 1];
    strncpy(name, dp->name, dp->name_len);
    name[dp->name_len] = '\0';

    if(dp->inode){
      MINODE *mip = iget(dev, dp->inode);
      ls_file(mip, name);
      iput(mip);
    }
    else{
      ls_dir(name);
    }

    cp += dp->rec_len;
    dp = (DIR*)cp;
  }
}

void ls(char pathname[])
{
    MINODE *mip = running->cwd;
    int ino = -1;

    if (pathname[0])
    {
        mip = path2inode(pathname);

        if (!mip) {
            printf("ls: %s does not exist\n", pathname);
            return;
        }
    }

    printf("i_block[0] = %d\n", mip->INODE.i_block[0]);
    printdir(mip->INODE);
    iput(mip);
}



int pwd(MINODE *wd)
{
  //-------   Step 1 for algo of pwd   -------//
  //  "if (wd == root) return;  //
  if (wd == root)
  {
    //This means I'm at root directory so my cwd is root or /
    //Also wd means working directory. Main.c calls this func and passes in running->cwd
    printf("/\n");
    return;
  }
  else
  {
    //As stated in the text this is a function that uses recursion on the directory minodes
    rpwd(wd);//Steps 2-6 are in this function
  }
  printf("\n");
}

int rpwd(MINODE *wd)
{
  //printf("PWD\n");
  if(wd == root)
  {
    return;
  }

  //-------   Step 2 for algo of pwd   -------//
  //  "from wd->INODE.i_block[0], get my ino and parent ino"  //
  char buf[BLKSIZE];
  char thisname[256];
  get_block(dev, wd->INODE.i_block[0], buf);

  //Testbook says to use this util.c func, int get_myino(MINODE *mip, int *parent_ino)
  //This should return the inode number of . and .. in parent inode

  //-------   Step 3 for algo of pwd   -------//
  //  "pip = iget(dev, parent ino)"
  int ino;
  int parent_ino = findino(wd, &ino);
  MINODE *pip = iget(dev, parent_ino);

  //-------   Step 4 for algo of pwd   -------//
  //  from pip->INODE.iblock[ ]; get my_name string by my_ino as LOCAL
    
  //Textbook says to use this util.c func, int findmyname(MINODE *pip, int myino, char myname[ ])
  //This should return the name string of the dir entry that is identified in the parent directory
  int error = findmyname(pip, ino, thisname);
  if(error == 0)
    printf("error in pwd %d\n", error);

//-------   Step 5 for algo of pwd   -------//
//  rpwd(pip); Recursively call rpwd(pip) with parent minode
  rpwd(pip);
  iput(pip);

//-------   Step 6 for algo of pwd   -------//
  printf("/%s", thisname);
  return;

}

