




extern MINODE *root, minode[NMINODE];
extern PROC proc[NPROC], *running;
extern int fd, dev, requests;
extern int ninodes, nblocks;
extern int bmap, imap, inodes_start, iblk;  // bitmap, inodes block numbers
extern char cmd[16], pathname[128], parameter[128];
extern int  requests, hits, inodes_start, misses;




int cat(char *filename)
{
	//printf("This is the filename: %s\n", filename);

	char mybuf[BLKSIZE], dummy = 0; //a null char at the end of mybuf[]
	int n;

	char flag[5];
	printf("Cat1\n");
	fgets(flag, 5, stdin);
	//1.) int fd = open filename for READ;
    int fd = open_file2(filename, 0);

	/*
	2.) while(   n = read(fd, mybuf[1024], 1024)   )
	{
		mybuf[n] = 0;    //as a null terminated string
		//printf("%s", mybuf);
		spit out chars from mybuf[ ] but handle \n properly;
	}
	*/

    //While the myread can read bytes of size BLKSIZE
    while (n = myread(fd, mybuf, BLKSIZE)) 
    {
        mybuf[n] = 0;
        char *cp = mybuf;
        while (*cp != '\0') //Since we are reading block of data at a time it is safe to assume that somewhere there is going to be an EOF character this should catch it
        {
            if (*cp == '\n') 
            {
                printf("\n"); //This catches the new lines and prints them
            } 
            else 
            {
                printf("%c", *cp); //This will print the retrieved character
            }
            cp++;//Moves forward in the mybuf stargae array for the next chunk of text
        }
    }
    

	//3.) close(fd);
    close_file(fd);
    return 0;

}


void cp(char *src, char* dest)
{
	//printf("This is the filename for copy: %s\n", filename);

// HOW TO cp ONE file:

// cp src dest:

// 1. fd = open src for READ;

// 2. gd = open dst for WR|CREAT; 

// NOTE: In the project, if dst for WRITE does not exist, your open() should
//       creat the dst file first, then open it for WRITE
	

// 3. while( n=read(fd, buf[ ], BLKSIZE) ){
//        write(gd, buf, n);  // notice the n in write()
//    }
}