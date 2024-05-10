


extern MINODE *root, minode[NMINODE];
extern PROC proc[NPROC], *running;
extern int fd, dev, requests;
extern int ninodes, nblocks;
extern int bmap, imap, inodes_start, iblk;  // bitmap, inodes block numbers
extern char cmd[16], pathname[128], parameter[128];
extern int  requests, hits, inodes_start, misses;


int read_file()
{
	
	//printf("read_file\n");

	int fd = 0;
	int nbytes = 0;
	char buf[BLKSIZE];


	/*
	From KC READ PROJECT HELP
	Assume file is opened for Read or read/write. ex. fd = 0 opened for read
	Ask for a fd and nbytes to read. ex. read 0 10 or read 0 123 means read mode 0 and 123 bytes 
	*/
	printf("Enter a fd\n");
	scanf("%d", &fd);

	printf("Enter the Number of bytes you want to read\n");
	scanf("%d", &nbytes);


	/*
	Verify that fd is indeed opened for Read or read/write
	then return myread(fd, buf, nbytes);
	*/
	if (fd < 0 || fd >= NFD) 
	{
		printf("File descriptor is not an opened fd\n");
		return -1;
	}

    if (running->fd[fd]->mode == 1 && running->fd[fd]->mode == 3) 
    {
    	printf("FD is not opened for READ(0) or READ/WRITE(2)\n");
    	return -1;
    }

    int count = 0;
    count = myread(fd, buf, nbytes);
    printf("Bytes read is = %d\n", count);
    return;
}


int myread2(int fd, char *buf, int nbytes)
{

    //1.) Assume that fd is opened for READ
	//2.) The offset in OFT points to the current byte position in the file from where we want to read numbBytes
	OFT *oftp = running->fd[fd];
	MINODE *mip = oftp->minodeptr;

	int start, blk, lbk;
	int indir_buf[BLKSIZE];
	int double_indir_buf[BLKSIZE] = { 0 };

	//3.) To the kernal, a file is just a sequence of continguous bytes, numbered from 0 to file_size -1. As the figure shows, the curret byte position, offset falls in a logical block or 'lbk', which is 
	//int lbk = offset / BLKSIZE;

	//The byte to start read in that logical block is 
	//int start = offset % BLKSIZE;

	//And the number of bytes remaining in the logical block is
	//int remain = BLKSIZE - start;

	//At this moment, the file has
	//int avil = file_size - offset;
	//bytes to read. These numbers are used in the read algo


	/*Compue LOGICAL BLOCK number lbk and start in that block from offset;
	lbk = oftp->offset / BLKSIZE;
	start = oftp->offset % BLKSIZE;*/

    lbk = oftp->offset / BLKSIZE;
    start = oftp->offset % BLKSIZE;
    int remain = BLKSIZE - start;
    int avil = mip->INODE.i_size - oftp->offset;


	//4.) myread() tries to read nbytes from fd to buf[], and return the actual number of bytes read.

    /*
    5.) 
		1.) 
		int count = 0;
		avil = file_size - OFT's offset //Number of bytes still available in file.
		char *cq = buf;
	*/
    int count = 0;
    char *cq = buf;



    if (nbytes > avil)
        nbytes = avil;//This will make it start at the end thus reading nothing

	/*
	2.)
	while(nbytes && avil)
	{
		
		//This is for direct blocks [0]-[11]
			if(lbk < 12) //This checks to see if lbk is a direct block
			{
				blk = mip->INODE.i_block[lbk]; //Mapp LOGICAL BLOCK to PHYSICAL BLOCK 
			}
			else if (lbk >= 12 && lbk < 256 + 12)
			{
				//Indirect blocks
			}
			else
			{
				//Double indirect blocks
			}
	*/
    while (nbytes && avil) {

        if (lbk < 12) {                             // direct blocks
            blk = mip->INODE.i_block[lbk];
        } else if (lbk >= 12 && lbk < 256 + 12) {   // indirect blocks
            // read block from i_block[12]
            get_block(mip->dev, mip->INODE.i_block[12], indir_buf);
            // offset it by 12 because 0-11 is direct 12
            blk = indir_buf[lbk - 12];
        } else {                                    // doubly indirect blocks
            // double indirect block is in i_block[13]
            get_block(mip->dev, mip->INODE.i_block[13], (char *)indir_buf);
            
            /*
            int ifactor = 256/128;
  			blk = (ino - 1) / ifactor + iblk;
  			offset = (ino - 1) % ifactor;
			
            int ifactor = 256/128;
            int section2 = (ifactor + iblk);
			*/
            //This is the optimization to the myread() function. instead of byte by byte its section by section based of of mailman algorithm to get section to the right position and then mod it to the correct place within eacah 256*256
            //1024/4 = 256
            int ifactor = 256/128;
            int section = (ifactor + iblk);
            lbk = lbk - section - 12; // reset lbk to 0 relatively
            blk = indir_buf[lbk / section]; // divide 'addresses'/indices by 256
            get_block(mip->dev, blk, double_indir_buf);
            // now modulus to get the correct mapping
            blk = double_indir_buf[lbk % section];
        }


		/*
		//get the data block into readbuf[BLKSIZE]
		get_block(mip->dev, blk, readbuf);
		*/
    	char readbuf[BLKSIZE];
    	// get data block into readbuf[BLKSIZE]
    	get_block(mip->dev, blk, readbuf);



    	/*
		//Copy from start to buf[], at most remain bytes in this block
		char *cp = readbuf + start;
		remain = BLKSIZE - start; //Number of bytes remain in readbuf[]
		*/
        // copy from start to buf[]
        char *cp = readbuf + start;
        remain = BLKSIZE - start;


		/*
		while (remain > 0)
		{
			*cq++ = *cp++; //This copies bytes from readbuf[] into buf[]
			oftp->offset++;  ///Advances offset
			count++;
			avil--;
			nbytes--;
			remain--;
			if(nbytes <= 0 || avil <= 0)
			{
				break;
			}
		}

		//if one data block is not enough, loop back to outer while for more...

		}
		printf("myread: read %d char from file descriptor %d\n", count fd);
		return count;  //Count is the actual number of bytes read
		*/
        while(remain > 0)
        {
        	*cq++ = *cp++; //This copies bytes from readbuf[] into buf[]
			oftp->offset++;  ///Advances offset
			count++;		//increments count for the number of bytes actually read
			avil--;			//Every time you go through there is less avliable
			nbytes--;		//Every time you get a byte into buf there is 1 less
			remain--;		//Every time you get a byte 1 less remains
			if(nbytes <= 0 || avil <= 0)//Stops when your at the end of the file or you read all the bytes you need
			{
				break;
			}
        }
    
		/*
        // copy entire sections at a time using the memcpy function
        if (nbytes <= remain) 
        {
            //the copies the memory address from cq to cp of the size of nbytes. Essentailly adding large sections of data from the read stream.
            memcpy(cq, cp, nbytes);

			//adjust all the amounts just like in the byte by byte implementation
            cq += nbytes;
            cp += nbytes;
            count += nbytes;
            oftp->offset += nbytes;
            avil -= nbytes; 
            remain -= nbytes;
            nbytes = 0;
        } 
        else 
        {
            //the copies the memory address from cq to cp of the size of nbytes. Essentailly adding large sections of data from the read stream.
            memcpy(cq, cp, remain);
			
			//adjust all the amounts just like in the byte by byte implementation
            cq += remain;
            cp += remain;
            count += remain;
            oftp->offset += remain;
            avil -= remain;
            nbytes -= remain; 
            remain = 0;
        }
		*/
	}
    printf("myread: read %d char from file descriptor %d, offset: %x\n", count, fd, oftp->offset); // FOR DEBUG
    return count;


}

int myread(int fd, char *buf, int nbytes)
{
    OFT *oftp = running->fd[fd];
    MINODE *mip = oftp->minodeptr;

    int start, blk, lbk;
    int indir_buf[BLKSIZE];
    int double_indir_buf[BLKSIZE] = { 0 };

    lbk = oftp->offset / BLKSIZE;
    start = oftp->offset % BLKSIZE;
    int avil = mip->INODE.i_size - oftp->offset;

    int count = 0;
    char *cq = buf;

    if (nbytes > avil)
        nbytes = avil;

    while (nbytes && avil) {
        if (lbk < 12) {                             // direct blocks
            blk = mip->INODE.i_block[lbk];
        } else if (lbk >= 12 && lbk < 256 + 12) {   // indirect blocks
            get_block(mip->dev, mip->INODE.i_block[12], indir_buf);
            blk = indir_buf[lbk - 12];
        } else {                                    // doubly indirect blocks
            get_block(mip->dev, mip->INODE.i_block[13], (char *)indir_buf);
            int iblk = mip->INODE.i_block[13];      // missing from original code
            int ifactor = 256/128;
            int section = (ifactor + iblk);
            lbk = lbk - section - 12;               // reset lbk to 0 relatively
            blk = indir_buf[lbk / section];         // divide 'addresses'/indices by 256
            get_block(mip->dev, blk, double_indir_buf);
            blk = double_indir_buf[lbk % section];
        }

        char readbuf[BLKSIZE];
        get_block(mip->dev, blk, readbuf);

        char *cp = readbuf + start;
        int remain = BLKSIZE - start;               // move this line up

        if (remain > avil)
            remain = avil;

        while (remain) {
            *cq++ = *cp++;
            oftp->offset++;
			count++;		//increments count for the number of bytes actually read
			avil--;			//Every time you go through there is less avliable
			nbytes--;		//Every time you get a byte into buf there is 1 less
			remain--;		//Every time you get a byte 1 less remains
			if(nbytes <= 0 || avil <= 0)//Stops when your at the end of the file or you read all the bytes you need
			{
				break;
			}
        }
    }
    printf("myread: read %d char from file descriptor %d, offset: %x\n", count, fd, oftp->offset); // FOR DEBUG
    return count;

}

