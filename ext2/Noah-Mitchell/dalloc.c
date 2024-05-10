int incFreeInodes(int dev)
{
  char buf[BLKSIZE];

  // inc free inodes count in SUPER and GD
  get_block(dev, 1, buf);
  sp = (SUPER *)buf;
  sp->s_free_inodes_count++;
  put_block(dev, 1, buf);

  get_block(dev, 2, buf);
  gp = (GD *)buf;
  gp->bg_free_inodes_count++;
  put_block(dev, 2, buf);
}

int idalloc(int dev, int ino)  // deallocate an ino number
{
  int i;  
  char buf[BLKSIZE];

  // return 0 if ino < 0 OR > ninodes

  // get inode bitmap block
  get_block(dev, bmap, buf);
  clr_bit(buf, ino - 1);

  // write buf back
  put_block(dev, imap, buf);

  // update free inode count in SUPER and GD
  incFreeInodes(dev);
}

int bdalloc(int dev, int blk) // deallocate a blk number
{
  int i;  
  char buf[BLKSIZE];

  // return 0 if blk < 0 OR > nblocks

  // get block bitmap block
  get_block(dev, bmap, buf);
  clr_bit(buf, blk - 1);

  // write buf back
  put_block(dev, bmap, buf);

  // update free block count in SUPER and GD
  get_block(dev, 1, buf);
  sp = (SUPER *)buf;
  sp->s_free_blocks_count++;
  put_block(dev, 1, buf);

  get_block(dev, 2, buf);
  gp = (GD *)buf;
  gp->bg_free_blocks_count++;
  put_block(dev, 2, buf);
}