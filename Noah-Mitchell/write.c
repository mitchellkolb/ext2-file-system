


void write_file()
{
	//printf("This is the filename for write: %s\n", filename);


	int fd = 0;
	char str[BLKSIZE], buf[BLKSIZE];
	int nbytes = 0;


	/*
	From KC READ PROJECT HELP
	1. ask for a fd   and   a text string to write;	*/
	printf("Enter a fd\n");
	scanf("%d", &fd);
	getchar();

	printf("Enter the string you want write to the FD\n");
  	fgets(str, BLKSIZE, stdin);


	/*
	2. Verify that fd is indeed opened for Write or read/write or append mode
	*/
	if (fd < 0 || fd >= NFD) 
	{
		printf("File descriptor is not opened an fd\n");
		return -1;
	}

    if (running->fd[fd]->mode != 1 && running->fd[fd]->mode != 2 && running->fd[fd]->mode != 3) 
    {
    	printf("FD is not opened for WRTIE(1) or READ/WRITE(2) or APPEND(3)\n");
    	return -1;
    }

    //3. copy the text string into a buf[] and get its length as nbytes.
    //	then return mywrite(fd, buf, nbytes);
    strcpy(buf, str);
    nbytes = strlen(buf);
    return mywrite(fd, buf, nbytes);
}


void mywrite(int fd, char buf[ ], int nbytes)
{
	printf("This is the # of bytes: %d\n", nbytes);

	OFT *oftp = running->fd[fd];
	MINODE *mip = oftp->minodeptr;

	int start, blk, lbk;
	int indir_buf[BLKSIZE];
	int double_indir_buf[BLKSIZE] = { 0 };
	lbk = oftp->offset / BLKSIZE;
    start = oftp->offset % BLKSIZE;
    int remain = BLKSIZE - start;
    int avil = mip->INODE.i_size - oftp->offset;
    int count = 0;
    char *cq = buf;

    while(nbytes > 0)
    {

    	//while (nbytes > 0 ){

		//compute LOGICAL BLOCK (lbk) and the startByte in that lbk:
		//lbk       = oftp->offset / BLKSIZE;
		//startByte = oftp->offset % BLKSIZE;

    	lbk = oftp->offset / BLKSIZE;
    	int startByte = oftp->offset % BLKSIZE;

	// I only show how to write DIRECT data blocks, you figure out how to 
	//     // write indirect and double-indirect blocks.

	    if (lbk < 12){                         // direct block
	       if (mip->INODE.i_block[lbk] == 0){   // if no data block yet

	          mip->INODE.i_block[lbk] = balloc(mip->dev);// MUST ALLOCATE a block
	       }
	       blk = mip->INODE.i_block[lbk];      // blk should be a disk block now
	    //else if (lbk >= 12 && lbk < 256 + 12){ // INDIRECT blocks 
	          // HELP INFO:
	          /*
	           if (i_block[12] == 0){
	              allocate a block for it;
	              zero out the block on disk !!!!
	           }
	           get i_block[12] into an int ibuf[256];
	           blk = ibuf[lbk - 12];
	           if (blk==0){
	               allocate a disk block;
	               record it in ibuf[lbk - 12];
	           }
	           write ibuf[ ] back to disk block i_block[12];
	           */
	    }


    }

	//   

	//     
	  
	//      

	//      /*******************************************************************
	//              linear lbk to double indirect block blk mapping:

	//      |0...12|13...267|268 ....|.......|.......| ..........      
	//      |Direct|Indirect|0       |       |       | ..........
	//         12     256    |  256     256     256 
	//                       |
	//                       ^
	//                   lbk-12-256
	//         --------------------------------------------------
	//          lbk -= (12+256);    // lbk begins from 0 
	//          lbk / 256;          // which double indirect block
	//          lbk % 256;          // which entry in that block 
	//      *******************************************************************/

	//      else{
	//             // double indirect blocks */
	//      }

	//      /* all cases come to here : write to the data block */
	//      get_block(mip->dev, blk, wbuf);   // read disk block into wbuf[ ]  
	//      char *cp = wbuf + startByte;      // cp points at startByte in wbuf[]
	//      remain = BLKSIZE - startByte;     // number of BYTEs remain in this block

	//      while (remain > 0){               // write as much as remain allows  
	//            *cp++ = *cq++;              // cq points at buf[ ]
	//            nbytes--; remain--;         // dec counts
	//            oftp->offset++;             // advance offset
	//            if (offset > INODE.i_size)  // especially for RW|APPEND mode
	//                mip->INODE.i_size++;    // inc file size (if offset > fileSize)
	//            if (nbytes <= 0) break;     // if already nbytes, break
	//      }
	//      put_block(mip->dev, blk, wbuf);   // write wbuf[ ] to disk
	     
	//      // loop back to outer while to write more .... until nbytes are written
	//   }

	//   mip->dirty = 1;       // mark mip dirty for iput() 
	//   printf("wrote %d char into file descriptor fd=%d\n", nbytes, fd);           
	//   return nbytes;
	// }

	




	printf("Successful write\n");

}

void mv(char *src, char* dest)
{
	//printf("This is the filename for mv: %s\n", filename);
// HOW TO mv (rename)
// mv src dest:

// 1. verify src exists; get its INODE in ==> you already know its dev
// 2. check whether src is on the same dev as src

//               CASE 1: same dev:
// 3. Hard link dst with src (i.e. same INODE number)
// 4. unlink src (i.e. rm src name from its parent directory and reduce INODE's
//                link count by 1).
                
//               CASE 2: not the same dev:
// 3. cp src to dst
// 4. unlink src

}


